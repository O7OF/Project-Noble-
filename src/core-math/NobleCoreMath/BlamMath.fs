namespace NobleCoreMath

open System

[<AutoOpen>]
module BlamMath =

    /// Represents a 2D point (x, y) in Blam! coordinate space
    [<Struct>]
    type RealPoint2D =
        { X: float32
          Y: float32 }
        static member Zero = { X = 0.0f; Y = 0.0f }
        static member Create(x, y) = { X = x; Y = y }

    /// Represents a 2D vector (i, j)
    [<Struct>]
    type RealVector2D =
        { I: float32
          J: float32 }
        static member Zero = { I = 0.0f; J = 0.0f }
        static member Create(i, j) = { I = i; J = j }

    /// Represents a 3D point (x, y, z) in Blam! world coordinates (Z-up, right-handed)
    /// Direct mapping of Bungie 'union real_point3d'
    [<Struct>]
    type RealPoint3D =
        { X: float32
          Y: float32
          Z: float32 }
        static member Zero = { X = 0.0f; Y = 0.0f; Z = 0.0f }
        static member Create(x, y, z) = { X = x; Y = y; Z = z }

        static member (+) (p: RealPoint3D, v: RealVector3D) =
            { X = p.X + v.I; Y = p.Y + v.J; Z = p.Z + v.K }

        static member (-) (p: RealPoint3D, v: RealVector3D) =
            { X = p.X - v.I; Y = p.Y - v.J; Z = p.Z - v.K }

        static member (-) (a: RealPoint3D, b: RealPoint3D) : RealVector3D =
            { I = a.X - b.X; J = a.Y - b.Y; K = a.Z - b.Z }

    /// Represents a 3D directional vector (i, j, k)
    /// Direct mapping of Bungie 'union real_vector3d'
    and [<Struct>] RealVector3D =
        { I: float32
          J: float32
          K: float32 }
        static member Zero = { I = 0.0f; J = 0.0f; K = 0.0f }
        static member UnitX = { I = 1.0f; J = 0.0f; K = 0.0f }
        static member UnitY = { I = 0.0f; J = 1.0f; K = 0.0f }
        static member UnitZ = { I = 0.0f; J = 0.0f; K = 1.0f }
        static member Create(i, j, k) = { I = i; J = j; K = k }

        static member (+) (a: RealVector3D, b: RealVector3D) =
            { I = a.I + b.I; J = a.J + b.J; K = a.K + b.K }

        static member (-) (a: RealVector3D, b: RealVector3D) =
            { I = a.I - b.I; J = a.J - b.J; K = a.K - b.K }

        static member (*) (v: RealVector3D, s: float32) =
            { I = v.I * s; J = v.J * s; K = v.K * s }

        static member (*) (s: float32, v: RealVector3D) = v * s

        static member (/) (v: RealVector3D, s: float32) =
            if abs s < 1e-7f then RealVector3D.Zero
            else { I = v.I / s; J = v.J / s; K = v.K / s }

    /// Represents a quaternion (i, j, k, w)
    /// Direct mapping of Bungie 'struct real32_quaternion' (real_math.obj)
    [<Struct>]
    type Real32Quaternion =
        { I: float32
          J: float32
          K: float32
          W: float32 }
        static member Identity = { I = 0.0f; J = 0.0f; K = 0.0f; W = 1.0f }
        static member Create(i, j, k, w) = { I = i; J = j; K = k; W = w }

    /// Represents a 4x3 affine transform matrix (orientation + translation)
    /// Direct mapping of Bungie 'struct real_matrix4x3'
    [<Struct>]
    type RealMatrix4x3 =
        { Scale: float32
          // Row 0 / Forward vector
          M00: float32; M01: float32; M02: float32
          // Row 1 / Left vector
          M10: float32; M11: float32; M12: float32
          // Row 2 / Up vector
          M20: float32; M21: float32; M22: float32
          // Row 3 / Translation (Position)
          Tx: float32; Ty: float32; Tz: float32 }

        static member Identity =
            { Scale = 1.0f
              M00 = 1.0f; M01 = 0.0f; M02 = 0.0f
              M10 = 0.0f; M11 = 1.0f; M12 = 0.0f
              M20 = 0.0f; M21 = 0.0f; M22 = 1.0f
              Tx = 0.0f; Ty = 0.0f; Tz = 0.0f }

    /// Represents a 4x4 projection matrix for rasterizer view calculations
    /// Direct mapping of Bungie 's_rasterizer_projection'
    [<Struct>]
    type RealMatrix4x4 =
        { M00: float32; M01: float32; M02: float32; M03: float32
          M10: float32; M11: float32; M12: float32; M13: float32
          M20: float32; M21: float32; M22: float32; M23: float32
          M30: float32; M31: float32; M32: float32; M33: float32 }

        static member Identity =
            { M00 = 1.0f; M01 = 0.0f; M02 = 0.0f; M03 = 0.0f
              M10 = 0.0f; M11 = 1.0f; M12 = 0.0f; M13 = 0.0f
              M20 = 0.0f; M21 = 0.0f; M22 = 1.0f; M23 = 0.0f
              M30 = 0.0f; M31 = 0.0f; M32 = 0.0f; M33 = 1.0f }

    /// Functional vector, quaternion and matrix operations mirroring Bungie's 'real_math.obj'
    module VectorOps =

        let inline dot (a: RealVector3D) (b: RealVector3D) : float32 =
            a.I * b.I + a.J * b.J + a.K * b.K

        let inline cross (a: RealVector3D) (b: RealVector3D) : RealVector3D =
            { I = a.J * b.K - a.K * b.J
              J = a.K * b.I - a.I * b.K
              K = a.I * b.J - a.J * b.I }

        let inline lengthSquared (v: RealVector3D) : float32 =
            dot v v

        let inline length (v: RealVector3D) : float32 =
            sqrt (lengthSquared v)

        let normalize (v: RealVector3D) : RealVector3D =
            let len = length v
            if len > 1e-7f then v / len else RealVector3D.Zero

        let inline distance (a: RealPoint3D) (b: RealPoint3D) : float32 =
            length (a - b)

        let inline lerp (a: RealVector3D) (b: RealVector3D) (t: float32) : RealVector3D =
            a + (b - a) * t

        let inline pointLerp (a: RealPoint3D) (b: RealPoint3D) (t: float32) : RealPoint3D =
            a + ((b - a) * t)

    module QuaternionOps =

        /// Functional implementation of Bungie's 'quaternion_normalize' (?quaternion_normalize@@YAXPAUreal32_quaternion@@@Z)
        let normalize (q: Real32Quaternion) : Real32Quaternion =
            let magSq = q.I * q.I + q.J * q.J + q.K * q.K + q.W * q.W
            if magSq > 1e-7f then
                let invMag = 1.0f / sqrt magSq
                { I = q.I * invMag; J = q.J * invMag; K = q.K * invMag; W = q.W * invMag }
            else
                Real32Quaternion.Identity

        /// Functional implementation of Bungie's 'quaternions_interpolate' (?quaternions_interpolate@@YAXPBUreal32_quaternion@@0MPAU1@@Z)
        /// Spherical Linear Interpolation (Slerp)
        let slerp (q1: Real32Quaternion) (q2: Real32Quaternion) (t: float32) : Real32Quaternion =
            let dot = q1.I * q2.I + q1.J * q2.J + q1.K * q2.K + q1.W * q2.W
            let q2Adj, dotAdj =
                if dot < 0.0f then
                    { I = -q2.I; J = -q2.J; K = -q2.K; W = -q2.W }, -dot
                else
                    q2, dot

            if dotAdj > 0.9995f then
                // Linear fallback for close orientations
                normalize {
                    I = q1.I + (q2Adj.I - q1.I) * t
                    J = q1.J + (q2Adj.J - q1.J) * t
                    K = q1.K + (q2Adj.K - q1.K) * t
                    W = q1.W + (q2Adj.W - q1.W) * t
                }
            else
                let theta0 = acos dotAdj
                let theta = theta0 * t
                let sinTheta = sin theta
                let sinTheta0 = sin theta0

                let s0 = cos theta - dotAdj * sinTheta / sinTheta0
                let s1 = sinTheta / sinTheta0

                { I = s0 * q1.I + s1 * q2Adj.I
                  J = s0 * q1.J + s1 * q2Adj.J
                  K = s0 * q1.K + s1 * q2Adj.K
                  W = s0 * q1.W + s1 * q2Adj.W }

        /// Creates a quaternion from Euler angles (yaw, pitch, roll in radians)
        /// Matching Halo Reach's rotation conventions in 'c_sandbox_engine::rotate_object'
        let fromEuler (yaw: float32) (pitch: float32) (roll: float32) : Real32Quaternion =
            let cy = cos (yaw * 0.5f)
            let sy = sin (yaw * 0.5f)
            let cp = cos (pitch * 0.5f)
            let sp = sin (pitch * 0.5f)
            let cr = cos (roll * 0.5f)
            let sr = sin (roll * 0.5f)

            normalize {
                W = cr * cp * cy + sr * sp * sy
                I = sr * cp * cy - cr * sp * sy
                J = cr * sp * cy + sr * cp * sy
                K = cr * cp * sy - sr * sp * cy
            }

        /// Converts a quaternion to Euler angles (yaw, pitch, roll in radians)
        let toEuler (q: Real32Quaternion) : (float32 * float32 * float32) =
            let sinrCosp = 2.0f * (q.W * q.I + q.J * q.K)
            let cosrCosp = 1.0f - 2.0f * (q.I * q.I + q.J * q.J)
            let roll = atan2 sinrCosp cosrCosp

            let sinp = 2.0f * (q.W * q.J - q.K * q.I)
            let pitch =
                if abs sinp >= 1.0f then
                    MathF.CopySign(MathF.PI / 2.0f, sinp)
                else
                    asin sinp

            let sinyCosp = 2.0f * (q.W * q.K + q.I * q.J)
            let cosyCosp = 1.0f - 2.0f * (q.J * q.J + q.K * q.K)
            let yaw = atan2 sinyCosp cosyCosp

            (yaw, pitch, roll)

        /// Rotates a 3D vector by a quaternion
        let rotateVector (q: Real32Quaternion) (v: RealVector3D) : RealVector3D =
            let qv = { I = q.I; J = q.J; K = q.K }
            let uv = VectorOps.cross qv v
            let uuv = VectorOps.cross qv uv
            v + (uv * (2.0f * q.W)) + (uuv * 2.0f)

        /// Rotates a 3D point around an origin point by a quaternion
        let rotatePoint (origin: RealPoint3D) (q: Real32Quaternion) (p: RealPoint3D) : RealPoint3D =
            let offset = p - origin
            let rotated = rotateVector q offset
            origin + rotated

    module MatrixOps =

        /// Creates a 4x3 matrix from position and orientation quaternion
        let fromPositionAndQuaternion (pos: RealPoint3D) (q: Real32Quaternion) : RealMatrix4x3 =
            let q = QuaternionOps.normalize q
            let xx = q.I * q.I
            let yy = q.J * q.J
            let zz = q.K * q.K
            let xy = q.I * q.J
            let xz = q.I * q.K
            let yz = q.J * q.K
            let wx = q.W * q.I
            let wy = q.W * q.J
            let wz = q.W * q.K

            { Scale = 1.0f
              M00 = 1.0f - 2.0f * (yy + zz)
              M01 = 2.0f * (xy - wz)
              M02 = 2.0f * (xz + wy)

              M10 = 2.0f * (xy + wz)
              M11 = 1.0f - 2.0f * (xx + zz)
              M12 = 2.0f * (yz - wx)

              M20 = 2.0f * (xz - wy)
              M21 = 2.0f * (yz + wx)
              M22 = 1.0f - 2.0f * (xx + yy)

              Tx = pos.X; Ty = pos.Y; Tz = pos.Z }

        /// Transforms a 3D point by a 4x3 matrix
        let transformPoint (m: RealMatrix4x3) (p: RealPoint3D) : RealPoint3D =
            { X = m.M00 * p.X + m.M01 * p.Y + m.M02 * p.Z + m.Tx
              Y = m.M10 * p.X + m.M11 * p.Y + m.M12 * p.Z + m.Ty
              Z = m.M20 * p.X + m.M21 * p.Y + m.M22 * p.Z + m.Tz }

        /// Transforms a 3D directional vector by a 4x3 matrix (ignoring translation)
        let transformVector (m: RealMatrix4x3) (v: RealVector3D) : RealVector3D =
            { I = m.M00 * v.I + m.M01 * v.J + m.M02 * v.K
              J = m.M10 * v.I + m.M11 * v.J + m.M12 * v.K
              K = m.M20 * v.I + m.M21 * v.J + m.M22 * v.K }

    module Projection =

        /// Project a 3D world space point to 2D screen coordinates
        /// Essential for rendering Forge object selection boxes, name tags, and overlay gizmos
        let worldToScreen (worldPoint: RealPoint3D) (viewProj: RealMatrix4x4) (screenWidth: float32) (screenHeight: float32) : (RealPoint2D * bool) =
            let x = worldPoint.X
            let y = worldPoint.Y
            let z = worldPoint.Z

            let clipX = x * viewProj.M00 + y * viewProj.M10 + z * viewProj.M20 + viewProj.M30
            let clipY = x * viewProj.M01 + y * viewProj.M11 + z * viewProj.M21 + viewProj.M31
            let clipW = x * viewProj.M03 + y * viewProj.M13 + z * viewProj.M23 + viewProj.M33

            if clipW <= 0.001f then
                // Point is behind the camera
                (RealPoint2D.Zero, false)
            else
                let ndcX = clipX / clipW
                let ndcY = clipY / clipW

                let screenX = (ndcX + 1.0f) * 0.5f * screenWidth
                let screenY = (1.0f - ndcY) * 0.5f * screenHeight

                ({ X = screenX; Y = screenY }, true)
