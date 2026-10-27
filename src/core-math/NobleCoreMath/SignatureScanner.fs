namespace NobleCoreMath

open System
open System.Globalization

module SignatureScanner =

    type PatternElement =
        | Exact of byte
        | Any

    type CompiledPattern =
        { PatternString: string
          Elements: PatternElement array
          Length: int }

    type MemoryPatch =
        { Name: string
          Offset: int64
          OriginalBytes: byte array
          ReplacementBytes: byte array
          Description: string }

    /// Compiles an IDA-style signature string (e.g. "48 8B 05 ? ? ? ? 48 85 C0")
    let compilePattern (patternStr: string) : CompiledPattern =
        let tokens = patternStr.Split([| ' '; '\t'; '\r'; '\n' |], StringSplitOptions.RemoveEmptyEntries)
        let elements =
            tokens
            |> Array.map (fun token ->
                if token = "?" || token = "??" then
                    Any
                else
                    match Byte.TryParse(token, NumberStyles.HexNumber, CultureInfo.InvariantCulture) with
                    | true, b -> Exact b
                    | false, _ -> Any)

        { PatternString = patternStr
          Elements = elements
          Length = elements.Length }

    /// Scans a byte array for the first occurrence of a compiled pattern
    let findFirstMatch (buffer: byte array) (pattern: CompiledPattern) : int64 option =
        let bufLen = buffer.Length
        let patLen = pattern.Length
        let elements = pattern.Elements

        if patLen = 0 || bufLen < patLen then
            None
        else
            let maxIdx = bufLen - patLen
            let mutable matchFound = false
            let mutable foundIdx = -1

            let mutable i = 0
            while i <= maxIdx && not matchFound do
                let mutable matches = true
                let mutable j = 0
                while j < patLen && matches do
                    match elements.[j] with
                    | Exact b ->
                        if buffer.[i + j] <> b then
                            matches <- false
                    | Any -> ()
                    j <- j + 1

                if matches then
                    matchFound <- true
                    foundIdx <- i
                else
                    i <- i + 1

            if matchFound then Some (int64 foundIdx) else None

    /// Scans a byte array for all occurrences of a compiled pattern
    let findAllMatches (buffer: byte array) (pattern: CompiledPattern) : int64 list =
        let bufLen = buffer.Length
        let patLen = pattern.Length
        let elements = pattern.Elements

        if patLen = 0 || bufLen < patLen then
            []
        else
            let maxIdx = bufLen - patLen
            let matches = ResizeArray<int64>()

            for i = 0 to maxIdx do
                let mutable isMatch = true
                let mutable j = 0
                while j < patLen && isMatch do
                    match elements.[j] with
                    | Exact b ->
                        if buffer.[i + j] <> b then
                            isMatch <- false
                    | Any -> ()
                    j <- j + 1

                if isMatch then
                    matches.Add(int64 i)

            Seq.toList matches

    /// Generates a NOP (0x90) patch for a specified range
    let createNopPatch (name: string) (offset: int64) (length: int) (originalBytes: byte array) : MemoryPatch =
        let nops = Array.create length 0x90uy
        { Name = name
          Offset = offset
          OriginalBytes = originalBytes
          ReplacementBytes = nops
          Description = $"NOP sled ({length} bytes) to neutralize instruction at 0x{offset:X}" }

    /// Generates a byte replacement patch
    let createBytePatch (name: string) (offset: int64) (originalBytes: byte array) (replacementBytes: byte array) (description: string) : MemoryPatch =
        { Name = name
          Offset = offset
          OriginalBytes = originalBytes
          ReplacementBytes = replacementBytes
          Description = description }

    /// Generates an x64 near jump (0xE9 rel32) patch
    let createRelativeJumpPatch (name: string) (fromOffset: int64) (toTarget: int64) (originalBytes: byte array) : MemoryPatch =
        // JMP opcode: 0xE9 + 4-byte relative signed offset
        let relOffset = int32 (toTarget - (fromOffset + 5L))
        let jumpBytes = Array.zeroCreate<byte> 5
        jumpBytes.[0] <- 0xE9uy
        let relBytes = BitConverter.GetBytes(relOffset)
        Array.Copy(relBytes, 0, jumpBytes, 1, 4)

        { Name = name
          Offset = fromOffset
          OriginalBytes = originalBytes
          ReplacementBytes = jumpBytes
          Description = $"x64 near JMP from 0x{fromOffset:X} to target 0x{toTarget:X} (Rel32: {relOffset})" }

    /// Catalog of known Reach Blam! signatures extracted from Omaha symbol database
    module ReachPatterns =

        /// Pattern for c_network_session_membership::set_slot_counts (?set_slot_counts@c_network_session_membership@@QAAXJJ_N@Z)
        /// Expands hardcoded 16-player lobby limits to 32/64 slots
        let SetSlotCounts =
            compilePattern "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B DA 48 8B F9 85 D2"

        /// Pattern for c_sandbox_engine::rotate_object (?rotate_object@c_sandbox_engine@@QBAXJJMMM@Z)
        /// Forge rotational axis unlock (pitch, yaw, roll continuous float overrides)
        let RotateObject =
            compilePattern "40 55 56 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 0F 29 74 24"

        /// Pattern for c_sandbox_engine::manipulate_object (?manipulate_object@c_sandbox_engine@@QBAXJJ@Z)
        let ManipulateObject =
            compilePattern "48 89 5C 24 ? 57 48 83 EC 30 8B FA 48 8B D9 85 D2"

        /// Pattern for main_loop_body (?main_loop_body@@YAXV?$c_flags@W4e_main_loop_body_part_flags@@...@@PAKPA_J1@Z)
        let MainLoopBody =
            compilePattern "48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 41 54 41 55 41 56 41 57 48 8D A8"

        /// Pattern for cache_files_copy_map (?cache_files_copy_map@@YA_NPBDW4e_cache_file_load_action@@@Z)
        let CacheFilesCopyMap =
            compilePattern "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 30 8B EA 48 8B F9"
