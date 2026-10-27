using System;
using System.IO;
using System.Text.Json;
using NobleLauncher.Models;

namespace NobleLauncher.Services
{
    public class ConfigurationService
    {
        private readonly string _configFilePath;

        public ConfigurationService()
        {
            var appDataDir = Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData), "ProjectNoble");
            Directory.CreateDirectory(appDataDir);
            _configFilePath = Path.Combine(appDataDir, "noble-launcher-config.json");
        }

        public LauncherConfig Load()
        {
            // First check local workspace directory
            if (File.Exists("noble-launcher-config.json"))
            {
                try
                {
                    var localJson = File.ReadAllText("noble-launcher-config.json");
                    var cfg = JsonSerializer.Deserialize<LauncherConfig>(localJson);
                    if (cfg != null) return cfg;
                }
                catch { }
            }

            // Fallback to %AppData%/ProjectNoble
            if (File.Exists(_configFilePath))
            {
                try
                {
                    var json = File.ReadAllText(_configFilePath);
                    var cfg = JsonSerializer.Deserialize<LauncherConfig>(json);
                    if (cfg != null) return cfg;
                }
                catch { }
            }

            return new LauncherConfig();
        }

        public void Save(LauncherConfig config)
        {
            var options = new JsonSerializerOptions { WriteIndented = true };
            var json = JsonSerializer.Serialize(config, options);

            try
            {
                File.WriteAllText(_configFilePath, json);
            }
            catch { }

            try
            {
                File.WriteAllText("noble-launcher-config.json", json);
            }
            catch { }
        }
    }
}
