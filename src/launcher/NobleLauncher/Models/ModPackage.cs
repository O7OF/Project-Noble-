using System;

namespace NobleLauncher.Models
{
    public enum ModSource
    {
        LocalDirectory,
        SteamWorkshop,
        CustomPackage
    }

    public class ModPackage
    {
        public string Id { get; set; } = Guid.NewGuid().ToString("N");
        public string Title { get; set; } = "Untitled Mod";
        public string Author { get; set; } = "Unknown";
        public string Version { get; set; } = "1.0.0";
        public string Description { get; set; } = string.Empty;
        public string DirectoryPath { get; set; } = string.Empty;
        public ModSource Source { get; set; } = ModSource.LocalDirectory;
        public bool IsEnabled { get; set; } = true;
        public int Priority { get; set; } = 1;
        public long TotalSizeBytes { get; set; }
        public int OverrideFileCount { get; set; }
        public string TargetMap { get; set; } = "All";
    }
}
