using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using NobleLauncher.Models;

namespace NobleLauncher.Services
{
    public class ModManager
    {
        public List<ModPackage> ScanMods(string? workshopDir, string? customModsDir)
        {
            var results = new List<ModPackage>();

            // 1. Scan Local Mods directory (./data/mods or custom)
            var localDirs = new List<string>
            {
                Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "data", "mods"),
                Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "ProjectNoble", "mods")
            };

            if (!string.IsNullOrWhiteSpace(customModsDir) && Directory.Exists(customModsDir))
            {
                localDirs.Insert(0, customModsDir);
            }

            foreach (var localDir in localDirs)
            {
                if (Directory.Exists(localDir))
                {
                    ScanDirectoryForMods(localDir, ModSource.LocalDirectory, results);
                }
            }

            // 2. Scan Steam Workshop directory
            if (!string.IsNullOrWhiteSpace(workshopDir) && Directory.Exists(workshopDir))
            {
                ScanDirectoryForMods(workshopDir, ModSource.SteamWorkshop, results);
            }

            // Load saved state (enabled / priority)
            LoadModState(results);

            return results.OrderBy(m => m.Priority).ThenBy(m => m.Title).ToList();
        }

        private void ScanDirectoryForMods(string rootDir, ModSource source, List<ModPackage> results)
        {
            try
            {
                var subDirs = Directory.GetDirectories(rootDir);
                foreach (var dir in subDirs)
                {
                    var dirName = Path.GetFileName(dir);
                    var mod = new ModPackage
                    {
                        Id = dirName,
                        DirectoryPath = dir,
                        Source = source,
                        Title = dirName
                    };

                    // Check for manifest.json or modinfo.json
                    var manifestPath = Path.Combine(dir, "manifest.json");
                    var modInfoPath = Path.Combine(dir, "modinfo.json");

                    string? jsonPath = File.Exists(manifestPath) ? manifestPath : (File.Exists(modInfoPath) ? modInfoPath : null);

                    if (jsonPath != null)
                    {
                        try
                        {
                            var json = File.ReadAllText(jsonPath);
                            using var doc = JsonDocument.Parse(json);
                            var root = doc.RootElement;

                            if (root.TryGetProperty("title", out var titleProp) || root.TryGetProperty("name", out titleProp))
                            {
                                mod.Title = titleProp.GetString() ?? dirName;
                            }
                            if (root.TryGetProperty("author", out var authorProp))
                            {
                                mod.Author = authorProp.GetString() ?? "Unknown";
                            }
                            if (root.TryGetProperty("version", out var versionProp))
                            {
                                mod.Version = versionProp.GetString() ?? "1.0.0";
                            }
                            if (root.TryGetProperty("description", out var descProp))
                            {
                                mod.Description = descProp.GetString() ?? string.Empty;
                            }
                            if (root.TryGetProperty("target_map", out var mapProp))
                            {
                                mod.TargetMap = mapProp.GetString() ?? "All";
                            }
                        }
                        catch { }
                    }

                    // Count override files & compute size
                    try
                    {
                        var files = Directory.GetFiles(dir, "*.*", SearchOption.AllDirectories);
                        mod.OverrideFileCount = files.Length;
                        long totalSize = 0;
                        foreach (var f in files)
                        {
                            totalSize += new FileInfo(f).Length;
                        }
                        mod.TotalSizeBytes = totalSize;
                    }
                    catch { }

                    // Only add if not already present
                    if (!results.Any(m => m.DirectoryPath.Equals(dir, StringComparison.OrdinalIgnoreCase)))
                    {
                        results.Add(mod);
                    }
                }
            }
            catch { }
        }

        public void SaveModState(IEnumerable<ModPackage> mods)
        {
            var stateList = mods.Select(m => new ModStateRecord
            {
                Id = m.Id,
                IsEnabled = m.IsEnabled,
                Priority = m.Priority
            }).ToList();

            try
            {
                var options = new JsonSerializerOptions { WriteIndented = true };
                var json = JsonSerializer.Serialize(stateList, options);

                var appData = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "ProjectNoble");
                Directory.CreateDirectory(appData);
                File.WriteAllText(Path.Combine(appData, "mod_loadorder.json"), json);
                File.WriteAllText("mod_loadorder.json", json);
            }
            catch { }
        }

        private void LoadModState(List<ModPackage> mods)
        {
            string? json = null;
            if (File.Exists("mod_loadorder.json"))
            {
                try { json = File.ReadAllText("mod_loadorder.json"); } catch { }
            }
            else
            {
                var appDataPath = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "ProjectNoble", "mod_loadorder.json");
                if (File.Exists(appDataPath))
                {
                    try { json = File.ReadAllText(appDataPath); } catch { }
                }
            }

            if (!string.IsNullOrEmpty(json))
            {
                try
                {
                    var records = JsonSerializer.Deserialize<List<ModStateRecord>>(json);
                    if (records != null)
                    {
                        var dict = records.ToDictionary(r => r.Id, StringComparer.OrdinalIgnoreCase);
                        foreach (var m in mods)
                        {
                            if (dict.TryGetValue(m.Id, out var rec))
                            {
                                m.IsEnabled = rec.IsEnabled;
                                m.Priority = rec.Priority;
                            }
                        }
                    }
                }
                catch { }
            }
        }

        private class ModStateRecord
        {
            public string Id { get; set; } = string.Empty;
            public bool IsEnabled { get; set; }
            public int Priority { get; set; }
        }
    }
}
