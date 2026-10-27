using System;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using NobleCoreMath;

namespace NobleLauncher.Tests
{
    [TestClass]
    public class CoreMathTests
    {
        [TestMethod]
        public void TestVectorOps_DotAndCross()
        {
            var v1 = BlamMath.RealVector3D.Create(1.0f, 0.0f, 0.0f);
            var v2 = BlamMath.RealVector3D.Create(0.0f, 1.0f, 0.0f);

            float dot = BlamMath.VectorOps.dot(v1, v2);
            Assert.AreEqual(0.0f, dot, 1e-5f);

            var cross = BlamMath.VectorOps.cross(v1, v2);
            Assert.AreEqual(0.0f, cross.I, 1e-5f);
            Assert.AreEqual(0.0f, cross.J, 1e-5f);
            Assert.AreEqual(1.0f, cross.K, 1e-5f);
        }

        [TestMethod]
        public void TestVectorOps_NormalizeAndLength()
        {
            var v = BlamMath.RealVector3D.Create(3.0f, 4.0f, 0.0f);
            float len = BlamMath.VectorOps.length(v);
            Assert.AreEqual(5.0f, len, 1e-5f);

            var norm = BlamMath.VectorOps.normalize(v);
            Assert.AreEqual(0.6f, norm.I, 1e-5f);
            Assert.AreEqual(0.8f, norm.J, 1e-5f);
            Assert.AreEqual(0.0f, norm.K, 1e-5f);
        }

        [TestMethod]
        public void TestQuaternionOps_FromEulerAndRotate()
        {
            // 90 degrees yaw (around Z axis)
            float yaw = MathF.PI / 2.0f;
            var q = BlamMath.QuaternionOps.fromEuler(yaw, 0.0f, 0.0f);

            var forward = BlamMath.RealVector3D.UnitX;
            var rotated = BlamMath.QuaternionOps.rotateVector(q, forward);

            // UnitX (1, 0, 0) rotated +90 deg around Z becomes UnitY (0, 1, 0)
            Assert.AreEqual(0.0f, rotated.I, 1e-4f);
            Assert.AreEqual(1.0f, rotated.J, 1e-4f);
            Assert.AreEqual(0.0f, rotated.K, 1e-4f);
        }

        [TestMethod]
        public void TestQuaternionOps_Slerp()
        {
            var q1 = BlamMath.Real32Quaternion.Identity;
            var q2 = BlamMath.QuaternionOps.fromEuler(MathF.PI / 2.0f, 0.0f, 0.0f);

            var mid = BlamMath.QuaternionOps.slerp(q1, q2, 0.5f);
            var (yaw, pitch, roll) = BlamMath.QuaternionOps.toEuler(mid);

            // Halfway slerp should be 45 degrees (PI / 4)
            Assert.AreEqual(MathF.PI / 4.0f, yaw, 1e-3f);
        }

        [TestMethod]
        public void TestMatrixOps_TransformPoint()
        {
            var pos = BlamMath.RealPoint3D.Create(10.0f, 20.0f, 30.0f);
            var q = BlamMath.Real32Quaternion.Identity;
            var m = BlamMath.MatrixOps.fromPositionAndQuaternion(pos, q);

            var localPoint = BlamMath.RealPoint3D.Create(1.0f, 2.0f, 3.0f);
            var worldPoint = BlamMath.MatrixOps.transformPoint(m, localPoint);

            Assert.AreEqual(11.0f, worldPoint.X, 1e-5f);
            Assert.AreEqual(22.0f, worldPoint.Y, 1e-5f);
            Assert.AreEqual(33.0f, worldPoint.Z, 1e-5f);
        }

        [TestMethod]
        public void TestProjection_WorldToScreen()
        {
            var worldPoint = BlamMath.RealPoint3D.Create(0.0f, 0.0f, 5.0f);
            var identityViewProj = BlamMath.RealMatrix4x4.Identity;

            var (screenPoint, visible) = BlamMath.Projection.worldToScreen(worldPoint, identityViewProj, 1920.0f, 1080.0f);

            Assert.IsTrue(visible);
            // Center of screen
            Assert.AreEqual(960.0f, screenPoint.X, 1.0f);
            Assert.AreEqual(540.0f, screenPoint.Y, 1.0f);
        }

        [TestMethod]
        public void TestSignatureScanner_FindMatchAndPatch()
        {
            byte[] dummyMemory = new byte[]
            {
                0x90, 0x90, 0x48, 0x89, 0x5C, 0x24, 0x10, 0x48, 0x89, 0x74, 0x24, 0x18, 0x57, 0xCC, 0xCC
            };

            var pattern = SignatureScanner.compilePattern("48 89 5C 24 ? 48 89 74 24 ? 57");
            var match = SignatureScanner.findFirstMatch(dummyMemory, pattern);

            Assert.IsTrue(Microsoft.FSharp.Core.OptionModule.IsSome(match));
            Assert.AreEqual(2L, match!.Value);

            var nopPatch = SignatureScanner.createNopPatch("TestNop", match.Value, 5, new byte[] { 0x48, 0x89, 0x5C, 0x24, 0x10 });
            Assert.AreEqual(5, nopPatch.ReplacementBytes.Length);
            Assert.AreEqual(0x90, nopPatch.ReplacementBytes[0]);
        }
    }
}
