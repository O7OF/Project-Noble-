using System;
using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NobleLauncher.Models;
using NobleLauncher.Services;

namespace NobleLauncher.Tests
{
    [TestClass]
    public class DiscoveryTests
    {
        [TestMethod]
        public void TestConfigurationService_DefaultValues()
        {
            var configService = new ConfigurationService();
            var config = configService.Load();

            Assert.IsNotNull(config);
            Assert.IsTrue(config.LaunchWindowed);
            Assert.IsTrue(config.EnableVfs);
            Assert.IsTrue(config.EnableExpandedLobby);
            Assert.AreEqual(11775, config.DedicatedPort);
        }

        [TestMethod]
        public void TestGameDiscoveryService_DetectsOrValidates()
        {
            var discovery = new GameDiscoveryService();
            var result = discovery.DiscoverInstallation();

            Assert.IsNotNull(result);
            // The result can be valid if MCC is present, or properly populated with status message if not
            Assert.IsFalse(string.IsNullOrEmpty(result.StatusMessage));
        }

        [TestMethod]
        public void TestProcessSupervisor_FindExecutable_ReturnsPath()
        {
            var supervisor = new ProcessSupervisor();
            var clientExe = supervisor.FindExecutable(dedicated: false);
            var dedicatedExe = supervisor.FindExecutable(dedicated: true);

            Assert.IsFalse(string.IsNullOrEmpty(clientExe));
            Assert.IsFalse(string.IsNullOrEmpty(dedicatedExe));
            Assert.IsTrue(clientExe.EndsWith("noble-client.exe", StringComparison.OrdinalIgnoreCase));
            Assert.IsTrue(dedicatedExe.EndsWith("noble-dedicated.exe", StringComparison.OrdinalIgnoreCase));
        }

        [TestMethod]
        public void TestModManager_ScanMods_DoesNotThrow()
        {
            var modManager = new ModManager();
            var mods = modManager.ScanMods(workshopDir: null, customModsDir: null);

            Assert.IsNotNull(mods);
        }
    }
}
