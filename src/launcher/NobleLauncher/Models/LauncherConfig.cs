using System;

namespace NobleLauncher.Models
{
    public class LauncherConfig
    {
        public string CustomMccPath { get; set; } = string.Empty;
        public string CustomDataDirectory { get; set; } = string.Empty;
        public bool LaunchWindowed { get; set; } = true;
        public int WindowWidth { get; set; } = 1280;
        public int WindowHeight { get; set; } = 720;
        public bool DisableVsync { get; set; } = false;
        public bool EnableVfs { get; set; } = true;
        public bool EnableExpandedLobby { get; set; } = true;
        public bool LaunchHeadlessDedicated { get; set; } = false;
        public int DedicatedPort { get; set; } = 11775;
        public string DedicatedRconPassword { get; set; } = "noble_admin_2026";
        public string LogLevel { get; set; } = "debug";
        public bool StartSuspendedForInjector { get; set; } = false;
        public string MasterServerUrl { get; set; } = "http://master.projectnoble.org:8080";
    }
}
