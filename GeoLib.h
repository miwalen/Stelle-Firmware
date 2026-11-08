#ifndef GeoLib_H
#define GeoLib_H

#include <Arduino.h>

#ifndef EPSILON
#define EPSILON 0.00001f
#endif

// Forward declarations
class Vector3;
class Quaternion;

// ============================================================
// Vector3
// ============================================================
class Vector3 {
public:
    float x, y, z;

    Vector3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vector3(float v) : x(v), y(v), z(v) {}
    Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vector3 axisX() { return Vector3(1.0f, 0.0f, 0.0f); }
    Vector3 axisY() { return Vector3(0.0f, 1.0f, 0.0f); }
    Vector3 axisZ() { return Vector3(0.0f, 0.0f, 1.0f); }

    float LengthSquared() const { return x * x + y * y + z * z; }
    float Length() const { return sqrtf(LengthSquared()); }

    Vector3 Normalized() const {
        float len = Length();
        if (len < EPSILON) return Vector3(0.0f, 0.0f, 0.0f);
        return Vector3(x / len, y / len, z / len);
    }
};

// --- Vector3 Helpers ---
inline Vector3 vecAdd(const Vector3 &a, const Vector3 &b) {
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
    return Vector3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
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
        Vector3 a = axis.Normalized();
        float half = angleRad * 0.5f;
        float s = sinf(half);
        return Quaternion(cosf(half), a.x * s, a.y * s, a.z * s);
    }

    Quaternion Conjugate() const { return Quaternion(w, -x, -y, -z); }
    static Quaternion Identity() { return Quaternion(1.0f, 0.0f, 0.0f, 0.0f); }

    float asRad(const Vector3& axis) const {
        Vector3 u = axis.Normalized();
        Vector3 q_vec = {x, y, z};

        float sinHalfRad = vecDot(q_vec, u);
        float cosHalfRad = w;
        return 2.0f * atan2f(sinHalfRad, cosHalfRad);
    }
};

// --- Quaternion Helpers ---
inline Quaternion quatMultiply(const Quaternion& a, const Quaternion& b) {
    return Quaternion(
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z,
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w
    );
}

inline Vector3 vecRotate(const Quaternion& q, const Vector3& v) {
    Quaternion qv(0.0f, v.x, v.y, v.z);
    Quaternion result = quatMultiply(quatMultiply(q, qv), q.Conjugate());
    return Vector3(result.x, result.y, result.z);
}

#endif // GeoLib_H