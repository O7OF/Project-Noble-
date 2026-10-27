using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace NobleLauncher.Services
{
    public class FirefightBlockInfo
    {
        public string FilePath { get; set; } = string.Empty;
        public string FileName { get; set; } = string.Empty;
        public int MapId { get; set; }
        public string MapName { get; set; } = string.Empty;
        public string EngineMapFile { get; set; } = string.Empty;
        public int Difficulty { get; set; }
        public string DifficultyName { get; set; } = string.Empty;
        public int GameModeTag { get; set; }
        public uint Skulls { get; set; }
        public int PlayerCount { get; set; }
        public string VariantName { get; set; } = string.Empty;
        public long FileSize { get; set; }
    }

    public static class FirefightBlockDecoder
    {
        private static readonly Dictionary<int, (string DisplayName, string MapFile)> MapLookup = new()
        {
            { 210, ("Beachhead", "ff10_prototype.map") },
            { 211, ("Corvette", "ff45_corvette.map") },
            { 212, ("Courtyard", "ff20_courtyard.map") },
            { 213, ("Glacier", "ff60_icecave.map") },
            { 214, ("Holdout", "ff70_holdout.map") },
            { 215, ("Outpost", "ff60_airview.map") },
            { 216, ("Overlook", "ff50_park.map") },
            { 217, ("Waterfront", "ff30_waterfront.map") },
            { 218, ("Unearthed", "ff_unearthed.map") },
            { 219, ("Installation 04", "cex_ff_halo.map") }
        };

        private static readonly string[] DifficultyNames = { "Easy", "Normal", "Heroic", "Legendary" };

        public static FirefightBlockInfo Decode(string filePath)
        {
            if (!File.Exists(filePath))
                throw new FileNotFoundException($"Firefight block file not found: {filePath}");

            byte[] data = File.ReadAllBytes(filePath);
            return DecodeFromBytes(data, filePath);
        }

        public static FirefightBlockInfo DecodeFromBytes(byte[] data, string sourcePath = "")
        {
            if (data == null || data.Length < 0x1D5D0)
                throw new ArgumentException($"Invalid launch block byte size: {data?.Length ?? 0}");

            var info = new FirefightBlockInfo
            {
                FilePath = sourcePath,
                FileName = Path.GetFileName(sourcePath),
                FileSize = data.Length
            };

            // Read Difficulty
            info.Difficulty = data[0x34];
            info.DifficultyName = (info.Difficulty >= 0 && info.Difficulty < DifficultyNames.Length)
                ? DifficultyNames[info.Difficulty]
                : $"Custom ({info.Difficulty})";

            // Read Skulls
            info.Skulls = BitConverter.ToUInt32(data, 0x3C);

            // Read Player count
            info.PlayerCount = data[0xE8];

            // Read MapId
            info.MapId = BitConverter.ToInt32(data, 0x1D5C0);
            if (info.MapId == 0)
            {
                info.MapId = BitConverter.ToInt32(data, 0x10);
            }

            // Read Mode Tag
            info.GameModeTag = BitConverter.ToInt32(data, 0x1D334);

            // Resolve Map Name & Map File
            if (MapLookup.TryGetValue(info.MapId, out var mapMeta))
            {
                info.MapName = mapMeta.DisplayName;
                info.EngineMapFile = mapMeta.MapFile;
            }
            else
            {
                info.MapName = $"Unknown Firefight ({info.MapId})";
                info.EngineMapFile = $"ff_{info.MapId}.map";
            }

            // Extract variant name from embedded variant block (+0x300)
            info.VariantName = ExtractVariantName(data, 0x300, 0xFC04);

            return info;
        }

        public static List<FirefightBlockInfo> ScanDirectory(string blocksDir)
        {
            var results = new List<FirefightBlockInfo>();
            if (!Directory.Exists(blocksDir)) return results;

            foreach (var file in Directory.GetFiles(blocksDir, "firefight_*.bin"))
            {
                try
                {
                    results.Add(Decode(file));
                }
                catch
                {
                    // Ignore non-block files
                }
            }

            results.Sort((a, b) =>
            {
                int mapComp = a.MapId.CompareTo(b.MapId);
                return mapComp != 0 ? mapComp : a.Difficulty.CompareTo(b.Difficulty);
            });

            return results;
        }

        private static string ExtractVariantName(byte[] data, int offset, int length)
        {
            if (offset + 128 > data.Length) return "Firefight Standard";

            // Look for UTF-16 or ASCII string headers in variant block
            for (int i = offset; i < offset + 512 && i + 32 < data.Length; i += 2)
            {
                // Check for UTF-16 string like "Firefight" or "Standard"
                if (data[i] >= 0x20 && data[i] <= 0x7E && data[i + 1] == 0 &&
                    data[i + 2] >= 0x20 && data[i + 2] <= 0x7E && data[i + 3] == 0)
                {
                    int strLen = 0;
                    while (i + strLen + 1 < data.Length && data[i + strLen] != 0 && data[i + strLen + 1] == 0 && strLen < 64)
                    {
                        strLen += 2;
                    }
                    if (strLen >= 6)
                    {
                        string candidate = Encoding.Unicode.GetString(data, i, strLen).Trim();
                        if (candidate.Length >= 3 && !candidate.StartsWith("_"))
                        {
                            return candidate;
                        }
                    }
                }
            }

            return "Firefight Standard";
        }
    }
}
