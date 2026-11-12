#pragma once

#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifndef EPSILON
#define EPSILON 0.00001f
#endif

namespace stelle {

// ============================================================
// Vector3
// ============================================================
class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    explicit Vector3(float v) : x(v), y(v), z(v) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    static Vector3 axisX() { return Vector3(1.0f, 0.0f, 0.0f); }
    static Vector3 axisY() { return Vector3(0.0f, 1.0f, 0.0f); }
    static Vector3 axisZ() { return Vector3(0.0f, 0.0f, 1.0f); }

    float LengthSquared() const { return x * x + y * y + z * z; }
    float Length() const { return sqrtf(LengthSquared()); }

    Vector3 Normalized() const {
        const float len = Length();
        if (len < EPSILON) return Vector3(0.0f, 0.0f, 0.0f);
        return Vector3(x / len, y / len, z / len);
    }
};

inline Vector3 vecAdd(const Vector3& a, const Vector3& b) {
    return Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
}
inline Vector3 vecSubtract(const Vector3& a, const Vector3& b) {
    return Vector3(a.x - b.x, a.y - b.y, a.z - b.z);
}
inline Vector3 vecScale(const Vector3& a, float s) {
    return Vector3(a.x * s, a.y * s, a.z * s);
}
inline float vecDot(const Vector3& a, const Vector3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline Vector3 vecCross(const Vector3& a, const Vector3& b) {
    return Vector3(a.y * b.z - a.z * b.y,
                   a.z * b.x - a.x * b.z,
                   a.x * b.y - a.y * b.x);
}
inline float vecDistance(const Vector3& a, const Vector3& b) {
    return vecSubtract(a, b).Length();
}

// ============================================================
// Quaternion
// ============================================================
class Quaternion {
public:
    float w, x, y, z;

    Quaternion() : w(1.0f), x(0.0f), y(0.0f), z(0.0f) {}
    Quaternion(float w_, float x_, float y_, float z_) : w(w_), x(x_), y(y_), z(z_) {}

    static Quaternion FromAxisAngle(const Vector3& axis, float angleRad) {
        const Vector3 a = axis.Normalized();
        const float half = angleRad * 0.5f;
        const float s = sinf(half);
        return Quaternion(cosf(half), a.x * s, a.y * s, a.z * s);
    }

    static Quaternion Identity() { return Quaternion(1.0f, 0.0f, 0.0f, 0.0f); }

    Quaternion Conjugate() const { return Quaternion(w, -x, -y, -z); }

    float asRad(const Vector3& axis) const {
        const Vector3 u = axis.Normalized();
        const float sinHalf = x * u.x + y * u.y + z * u.z;
        return 2.0f * atan2f(sinHalf, w);
    }
};

inline Quaternion quatMultiply(const Quaternion& a, const Quaternion& b) {
    return Quaternion(
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w);
}

inline Vector3 vecRotate(const Quaternion& q, const Vector3& v) {
    const Quaternion qv(0.0f, v.x, v.y, v.z);
    const Quaternion r = quatMultiply(quatMultiply(q, qv), q.Conjugate());
    return Vector3(r.x, r.y, r.z);
}

} // namespace stelle
