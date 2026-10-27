using System.IO;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NobleLauncher.Services;

namespace NobleLauncher.Tests
{
    [TestClass]
    public class FirefightDecoderTests
    {
        [TestMethod]
        public void TestDecodeFirefight210()
        {
            string dir = AppContext.BaseDirectory;
            string blocksDir = "";
            while (!string.IsNullOrEmpty(dir))
            {
                string candidate = Path.Combine(dir, "data", "blocks");
                if (Directory.Exists(candidate))
                {
                    blocksDir = candidate;
                    break;
                }
                dir = Path.GetDirectoryName(dir);
            }

            if (!Directory.Exists(blocksDir))
            {
                Assert.Inconclusive($"Blocks directory not found at {blocksDir}");
                return;
            }

            string file210 = Path.Combine(blocksDir, "firefight_210_d1.bin");
            if (!File.Exists(file210))
            {
                Assert.Inconclusive("firefight_210_d1.bin not found");
                return;
            }

            var info = FirefightBlockDecoder.Decode(file210);

            Assert.IsNotNull(info);
            Assert.AreEqual(210, info.MapId);
            Assert.AreEqual("Beachhead", info.MapName);
            Assert.AreEqual("ff10_prototype.map", info.EngineMapFile);
            Assert.AreEqual(1, info.Difficulty);
            Assert.AreEqual("Normal", info.DifficultyName);
            Assert.AreEqual(1035, info.GameModeTag);
            Assert.AreEqual(180016, info.FileSize);
        }

        [TestMethod]
        public void TestScanAllFirefightBlocks()
        {
            string dir = AppContext.BaseDirectory;
            string blocksDir = "";
            while (!string.IsNullOrEmpty(dir))
            {
                string candidate = Path.Combine(dir, "data", "blocks");
                if (Directory.Exists(candidate))
                {
                    blocksDir = candidate;
                    break;
                }
                dir = Path.GetDirectoryName(dir);
            }

            if (!Directory.Exists(blocksDir))
            {
                Assert.Inconclusive($"Blocks directory not found at {blocksDir}");
                return;
            }

            var blocks = FirefightBlockDecoder.ScanDirectory(blocksDir);
            Assert.IsTrue(blocks.Count >= 10, $"Expected at least 10 firefight blocks, found {blocks.Count}");

            // Verify map 219 (Installation 04)
            var inst04 = blocks.Find(b => b.MapId == 219);
            Assert.IsNotNull(inst04);
            Assert.AreEqual("Installation 04", inst04.MapName);
            Assert.AreEqual("cex_ff_halo.map", inst04.EngineMapFile);
        }
    }
}
