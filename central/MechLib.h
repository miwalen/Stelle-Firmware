#ifndef MechLib_H
#define MechLib_H

#include <Arduino.h>
#include <math.h>
#include <vector>
#include "GeoLib.h"
#include <atomic>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
// Button — Menangani logika input digital (Press, Release, Hold)
// ============================================================
class Button {
private:
    bool     isInverted   = false;
    uint16_t debounceMs   = 20;

    // Hanya disentuh task yang memanggil update()
    bool     lastRaw      = false;
    uint32_t lastChangeMs = 0;

    // Dibaca/ditulis lintas task
    std::atomic<bool>     stableState{false};
    std::atomic<uint8_t>  pressCount{0};     // press yang belum dibaca
    std::atomic<uint8_t>  releaseCount{0};   // release yang belum dibaca

    static bool consumeOne(std::atomic<uint8_t>& counter);

public:
    void setInvert(bool invert);
    void setDebounceMs(uint16_t ms);

    void update(bool rawInput, uint32_t nowMs);  // panggil periodik

    bool isPressed();    // true sekali per event press
    bool isReleased();   // true sekali per event release
    bool isDown() const; // status tombol sekarang
};

// ============================================================
// EncoderJoints — Menangani sensor encoder, posisi, dan rotasi
// ============================================================
class EncoderJoints {
private:
    static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);

public:
    float analogInput = 0.0f;
    float rotationoffset = 0.0f; // radian

    uint16_t analoginputmaxmin[2] = {0, 1023};
    float radmaxmin[2] = {-2.0f * M_PI, 2.0f * M_PI};

    Vector3 axis             = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 offsetPosition   = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 localPosition    = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 worldPosition    = Vector3(0.0f, 0.0f, 0.0f);
    Quaternion worldRotation = Quaternion();

    // Setter
    void setAnalogInput(float value);
    void setAngleRad(float angleRad);
    void setAngleDegree(float angleDeg);
    void setMaxMin(float minRad, float maxRad);
    void setAnalogMaxMin(float minVal, float maxVal);
    void setOffsetDegree(float degreeOffset);
    void setOffsetRad(float radOffset);
    void setAxis(const Vector3& a);
    void setAxis(float x, float y, float z);
    void setOffsetPosition(const Vector3& pos);
    void setOffsetPosition(float x, float y, float z);

    // Getter & kinematics
    float getDegreeOffset();
    float getRadOffset();
    float asRawRad();
    float asRawDegree();
    float asDegree();
    float asRad();
    Quaternion asQuat();
};

// ============================================================
// InputChannels — Menangani buffer data input dari remote/sensor
// ============================================================
class InputChannels {
public:
    static const int SIZE = 7;

private:
    float data[SIZE] = {0.0f};

public:
    // Setter
    void set(int index, float value);
    void set(float v0, float v1, float v2, float v3, float v4, float v5, float v6);
    void set(const float* arr);

    // Getter & binding
    float getChannel(int index);
    float* bindChannels();
    float* bindChannel(int index);

    // Utilities
    void clear();
    bool isPressed(int index, float threshold = 0.5f);
    void applyDeadzone(int index, float deadzone);
    void applyDeadzoneAll(float deadzone);
};

// ============================================================
// KinematicChain — Model rantai kinematika serial dari kumpulan joint
// ============================================================
class KinematicChain {
private:
    std::vector<EncoderJoints> joints;

    bool valid(int index);

public:
    KinematicChain();

    // Joint management
    int addJoint(const EncoderJoints& joint);
    int addJoint();
    int getJointCount();

    // Config satu baris
    int addJointConfig(const Vector3& axis, const Vector3& offsetPos,
                       float offsetDegree, float minRad, float maxRad,
                       float analogMin, float analogMax);
    int addJointConfig(float ax, float ay, float az,
                       float px, float py, float pz,
                       float offsetDegree, float minRad, float maxRad,
                       float analogMin, float analogMax);

    void setJointConfig(int index, const Vector3& axis, const Vector3& offsetPos,
                        float offsetDegree, float minRad, float maxRad,
                        float analogMin, float analogMax);
    void setJointConfig(int index,
                        float ax, float ay, float az,
                        float px, float py, float pz,
                        float offsetDegree, float minRad, float maxRad,
                        float analogMin, float analogMax);

    // Config per joint
    void setJointAxis(int index, const Vector3& axis);
    void setJointAxis(int index, float x, float y, float z);
    void setJointOffsetPosition(int index, const Vector3& pos);
    void setJointOffsetPosition(int index, float x, float y, float z);
    void setJointOffsetDegree(int index, float degreeOffset);
    void setJointOffsetRad(int index, float radOffset);
    void setJointMaxMin(int index, float minRad, float maxRad);
    void setJointAnalogMaxMin(int index, float minVal, float maxVal);

    // Input
    void setAnalogInput(int index, float value);
    void setJointAngleRad(int index, float angleRad);
    void setJointAngleDegree(int index, float angleDeg);

    // Compute
    void compute();

    // Getter
    Quaternion getJointQuat(int index);
    Vector3 getJointPos(int index);
    EncoderJoints* getJoint(int index);
    float getJointAngleDegree(int index); // Menambahkan getter derajat
    float getJointAngleRad(int index);    // Menambahkan getter radian
};

using KinematicArm = KinematicChain;

#endif // MechLib_H