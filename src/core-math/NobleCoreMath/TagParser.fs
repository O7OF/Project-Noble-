namespace NobleCoreMath

open System
open System.IO
open System.Text

module TagParser =

    type CacheMapType =
        | SinglePlayer = 0
        | Multiplayer = 1
        | MainMenu = 2
        | Shared = 3
        | SinglePlayerShared = 4
        | Unknown = 99

    type CacheFileHeader =
        { Signature: string
          EngineVersion: int32
          FileLength: int64
          TagIndexOffset: int64
          VirtualBaseAddress: uint64
          ScenarioName: string
          BuildString: string
          MapType: CacheMapType
          Checksum: uint32 }

    type BlfChunkHeader =
        { ChunkMagic: string
          ChunkLength: int32
          MajorVersion: int16
          MinorVersion: int16 }

    type VariantMetadata =
        { VariantName: string
          Description: string
          Author: string
          ChunkType: string
          IsMapVariant: bool
          IsGameVariant: bool }

    /// Parses a Halo: Reach .map cache file header
    let parseMapHeader (filePath: string) : Result<CacheFileHeader, string> =
        if not (File.Exists filePath) then
            Error $"File '{filePath}' does not exist."
        else
            try
                use stream = File.OpenRead filePath
                use reader = new BinaryReader(stream)

                // Read magic signature (4 bytes)
                let magicBytes = reader.ReadBytes 4
                let magicAscii = Encoding.ASCII.GetString magicBytes
                let magicReversed = Encoding.ASCII.GetString(Array.rev magicBytes)

                let isValidMagic = magicAscii = "head" || magicAscii = "daeh" || magicReversed = "head"
                if not isValidMagic then
                    Error $"Invalid cache file header signature: '{magicAscii}'. Expected 'head' or 'daeh'."
                else
                    let version = reader.ReadInt32()
                    let fileLength = int64 (reader.ReadUInt32())

                    // Seek to known offsets in Reach .map header
                    // Tag index virtual offset is at 0x10
                    stream.Seek(0x10L, SeekOrigin.Begin) |> ignore
                    let tagIndexOffset = int64 (reader.ReadUInt32())

                    // Virtual base address is at 0x18
                    stream.Seek(0x18L, SeekOrigin.Begin) |> ignore
                    let virtualBase = reader.ReadUInt64()

                    // Build string is located at 0x118 (up to 32 bytes)
                    stream.Seek(0x118L, SeekOrigin.Begin) |> ignore
                    let buildBytes = reader.ReadBytes 32
                    let buildStr = 
                        let nullIdx = Array.IndexOf(buildBytes, 0uy)
                        let count = if nullIdx >= 0 then nullIdx else buildBytes.Length
                        Encoding.ASCII.GetString(buildBytes, 0, count)

                    // Scenario path is located at 0x138 (up to 256 bytes)
                    stream.Seek(0x138L, SeekOrigin.Begin) |> ignore
                    let scenarioBytes = reader.ReadBytes 256
                    let scenarioStr =
                        let nullIdx = Array.IndexOf(scenarioBytes, 0uy)
                        let count = if nullIdx >= 0 then nullIdx else scenarioBytes.Length
                        Encoding.ASCII.GetString(scenarioBytes, 0, count)

                    // Map Type at 0x238
                    stream.Seek(0x238L, SeekOrigin.Begin) |> ignore
                    let rawMapType = reader.ReadInt32()
                    let mapType = 
                        if Enum.IsDefined(typeof<CacheMapType>, rawMapType) then
                            enum<CacheMapType> rawMapType
                        else
                            CacheMapType.Unknown

                    Ok {
                        Signature = magicAscii
                        EngineVersion = version
                        FileLength = fileLength
                        TagIndexOffset = tagIndexOffset
                        VirtualBaseAddress = virtualBase
                        ScenarioName = scenarioStr
                        BuildString = buildStr
                        MapType = mapType
                        Checksum = 0u
                    }
            with ex ->
                Error $"Failed to parse map header: {ex.Message}"

    /// Parses a Bungie BLF (.bin / .mvar) header container
    let parseBlfHeader (stream: Stream) : Result<VariantMetadata, string> =
        try
            use reader = new BinaryReader(stream, Encoding.UTF8, leaveOpen = true)
            let magic = reader.ReadBytes 4
            let magicStr = Encoding.ASCII.GetString magic

            if magicStr <> "_blf" then
                Error $"Invalid BLF header signature: '{magicStr}'. Expected '_blf'."
            else
                // Read chunks until 'chdr' (Content Header), 'mpvr' (Map Variant), or 'gvar' (Game Variant)
                let rec readChunks (acc: VariantMetadata) =
                    if stream.Position >= stream.Length - 16L then
                        acc
                    else
                        let chunkMagicBytes = reader.ReadBytes 4
                        let chunkMagic = Encoding.ASCII.GetString chunkMagicBytes
                        let chunkLen = reader.ReadInt32()
                        let _majVer = reader.ReadInt16()
                        let _minVer = reader.ReadInt16()

                        if chunkLen <= 0 || int64 chunkLen > (stream.Length - stream.Position) then
                            acc
                        else
                            let nextPos = stream.Position + int64 (chunkLen - 12)
                            match chunkMagic with
                            | "chdr" ->
                                // Content Header contains UTF-16 variant name and description
                                stream.Seek(0x18L, SeekOrigin.Current) |> ignore
                                let nameBytes = reader.ReadBytes 64
                                let name = Encoding.Unicode.GetString(nameBytes).TrimEnd('\000')
                                stream.Seek(nextPos, SeekOrigin.Begin) |> ignore
                                readChunks { acc with VariantName = name }
                            | "mpvr" ->
                                stream.Seek(nextPos, SeekOrigin.Begin) |> ignore
                                readChunks { acc with ChunkType = "mpvr"; IsMapVariant = true }
                            | "gvar" ->
                                stream.Seek(nextPos, SeekOrigin.Begin) |> ignore
                                readChunks { acc with ChunkType = "gvar"; IsGameVariant = true }
                            | _ ->
                                stream.Seek(nextPos, SeekOrigin.Begin) |> ignore
                                readChunks acc

                let initial = {
                    VariantName = "Unnamed Variant"
                    Description = ""
                    Author = ""
                    ChunkType = "unknown"
                    IsMapVariant = false
                    IsGameVariant = false
                }

                Ok (readChunks initial)
        with ex ->
            Error $"BLF parser exception: {ex.Message}"

    /// Reads metadata from a .mvar or .bin file
    let parseVariantFile (filePath: string) : Result<VariantMetadata, string> =
        if not (File.Exists filePath) then
            Error $"File '{filePath}' does not exist."
        else
            try
                use stream = File.OpenRead filePath
                parseBlfHeader stream
            with ex ->
                Error $"Failed to parse variant file: {ex.Message}"
