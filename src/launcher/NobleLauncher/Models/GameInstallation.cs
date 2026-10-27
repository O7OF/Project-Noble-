using System;
using System.IO;

namespace NobleLauncher.Models
{
    public class GameInstallation
    {
        public bool IsValid { get; set; }
        public string MccRootPath { get; set; } = string.Empty;
        public string ReachDllPath { get; set; } = string.Empty;
        public string MapsDirectory { get; set; } = string.Empty;
        public string FmodDirectory { get; set; } = string.Empty;
        public string WorkshopDirectory { get; set; } = string.Empty;
        public string StatusMessage { get; set; } = string.Empty;
        public long ReachDllSizeBytes { get; set; }
        public int MapCount { get; set; }
        public bool HasMainMenuMap { get; set; }
        public bool HasReachDll { get; set; }
        public uint SteamAppId { get; set; } = 976730;

        public static GameInstallation Invalid(string reason)
        {
            return new GameInstallation
            {
                IsValid = false,
                StatusMessage = reason
            };
        }
    }
}
