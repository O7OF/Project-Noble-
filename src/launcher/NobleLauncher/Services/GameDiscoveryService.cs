using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using Microsoft.Win32;
using NobleLauncher.Models;

namespace NobleLauncher.Services
{
    public class GameDiscoveryService
    {
        private const uint MccAppId = 976730;

        public GameInstallation DiscoverInstallation(string? customPath = null)
        {
            if (!string.IsNullOrWhiteSpace(customPath) && Directory.Exists(customPath))
            {
                var customInstall = ValidatePath(customPath);
                if (customInstall.IsValid)
                {
                    return customInstall;
                }
            }

            // 1. Direct Registry Query for Steam App 976730
            try
            {
                using var key = Registry.LocalMachine.OpenSubKey($@"SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Steam App {MccAppId}");
                if (key != null)
                {
                    var installLoc = key.GetValue("InstallLocation") as string;
                    if (!string.IsNullOrEmpty(installLoc) && Directory.Exists(installLoc))
                    {
                        var install = ValidatePath(installLoc);
                        if (install.IsValid)
                        {
                            return install;
                        }
                    }
                }
            }
            catch
            {
                // Registry access can be restricted; continue fallback
            }

            // 2. Discover Steam installation root and parse libraryfolders.vdf
            var steamLibraries = GetSteamLibraries();
            foreach (var library in steamLibraries)
            {
                var candidate = Path.Combine(library, "steamapps", "common", "Halo The Master Chief Collection");
                if (Directory.Exists(candidate))
                {
                    var install = ValidatePath(candidate);
                    if (install.IsValid)
                    {
                        return install;
                    }
                }
            }

            // 3. Fallback standard drive probes
            string[] standardProbes =
            {
                @"C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection",
                @"C:\Steam\steamapps\common\Halo The Master Chief Collection",
                @"D:\Steam\steamapps\common\Halo The Master Chief Collection",
                @"D:\SteamLibrary\steamapps\common\Halo The Master Chief Collection",
                @"E:\SteamLibrary\steamapps\common\Halo The Master Chief Collection",
                @"F:\SteamLibrary\steamapps\common\Halo The Master Chief Collection"
            };

            foreach (var probe in standardProbes)
            {
                if (Directory.Exists(probe))
                {
                    var install = ValidatePath(probe);
                    if (install.IsValid)
                    {
                        return install;
                    }
                }
            }

            return GameInstallation.Invalid("Halo: The Master Chief Collection (AppID 976730) with Halo: Reach DLC could not be located.");
        }

        public GameInstallation ValidatePath(string rootPath)
        {
            var install = new GameInstallation
            {
                MccRootPath = Path.GetFullPath(rootPath)
            };

            var reachDll = Path.Combine(install.MccRootPath, "haloreach", "haloreach.dll");
            var mapsDir = Path.Combine(install.MccRootPath, "haloreach", "maps");
            var fmodDir = Path.Combine(install.MccRootPath, "haloreach", "fmod");

            install.ReachDllPath = reachDll;
            install.MapsDirectory = mapsDir;
            install.FmodDirectory = fmodDir;

            // Determine Steam Workshop directory
            try
            {
                var parentDir = Directory.GetParent(install.MccRootPath); // "common"
                var steamAppsDir = parentDir?.Parent?.FullName;          // "steamapps"
                if (!string.IsNullOrEmpty(steamAppsDir))
                {
                    var workshopDir = Path.Combine(steamAppsDir, "workshop", "content", MccAppId.ToString());
                    if (Directory.Exists(workshopDir))
                    {
                        install.WorkshopDirectory = workshopDir;
                    }
                }
            }
            catch
            {
                // Workshop dir resolution is optional
            }

            if (!File.Exists(reachDll))
            {
                install.IsValid = false;
                install.StatusMessage = "haloreach.dll was not found. Please ensure the Halo: Reach DLC is installed via Steam.";
                return install;
            }

            install.HasReachDll = true;
            var dllInfo = new FileInfo(reachDll);
            install.ReachDllSizeBytes = dllInfo.Length;

            if (!Directory.Exists(mapsDir))
            {
                install.IsValid = false;
                install.StatusMessage = "Halo: Reach maps directory (/haloreach/maps/) is missing.";
                return install;
            }

            var mainMenuMap = Path.Combine(mapsDir, "mainmenu.map");
            install.HasMainMenuMap = File.Exists(mainMenuMap);

            var mapFiles = Directory.GetFiles(mapsDir, "*.map", SearchOption.TopDirectoryOnly);
            install.MapCount = mapFiles.Length;

            if (!install.HasMainMenuMap)
            {
                install.IsValid = false;
                install.StatusMessage = "mainmenu.map was not found in the maps directory. Game installation may be corrupted.";
                return install;
            }

            install.IsValid = true;
            install.StatusMessage = $"Verified: haloreach.dll ({dllInfo.Length / (1024 * 1024)} MB) and {install.MapCount} maps ready.";
            return install;
        }

        private List<string> GetSteamLibraries()
        {
            var libraries = new List<string>();

            string? steamPath = null;
            try
            {
                using var cuKey = Registry.CurrentUser.OpenSubKey(@"Software\Valve\Steam");
                steamPath = cuKey?.GetValue("SteamPath") as string;
            }
            catch { }

            if (string.IsNullOrEmpty(steamPath))
            {
                try
                {
                    using var lmKey = Registry.LocalMachine.OpenSubKey(@"SOFTWARE\WOW6432Node\Valve\Steam");
                    steamPath = lmKey?.GetValue("InstallPath") as string;
                }
                catch { }
            }

            if (!string.IsNullOrEmpty(steamPath) && Directory.Exists(steamPath))
            {
                libraries.Add(steamPath);

                var vdfPath = Path.Combine(steamPath, "steamapps", "libraryfolders.vdf");
                if (File.Exists(vdfPath))
                {
                    try
                    {
                        var content = File.ReadAllText(vdfPath);
                        // Matches "path" "[Path]" format
                        var matches = Regex.Matches(content, @"""path""\s+""([^""]+)""", RegexOptions.IgnoreCase);
                        foreach (Match match in matches)
                        {
                            if (match.Groups.Count > 1)
                            {
                                var parsedPath = match.Groups[1].Value.Replace(@"\\", @"\");
                                if (Directory.Exists(parsedPath) && !libraries.Contains(parsedPath))
                                {
                                    libraries.Add(parsedPath);
                                }
                            }
                        }

                        // Matches legacy numeric index format: "1" "[Path]"
                        var legacyMatches = Regex.Matches(content, @"""\d+""\s+""([^""]+)""", RegexOptions.IgnoreCase);
                        foreach (Match match in legacyMatches)
                        {
                            if (match.Groups.Count > 1)
                            {
                                var parsedPath = match.Groups[1].Value.Replace(@"\\", @"\");
                                if (Directory.Exists(parsedPath) && !libraries.Contains(parsedPath))
                                {
                                    libraries.Add(parsedPath);
                                }
                            }
                        }
                    }
                    catch
                    {
                        // Continue with primary steamPath
                    }
                }
            }

            return libraries;
        }
    }
}
