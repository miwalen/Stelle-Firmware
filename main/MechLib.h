#pragma once

#include <atomic>
#include <cmath>
#include <cstdint>
#include <vector>

#include "GeoLib.h"

namespace stelle {

// ============================================================
// Button - logika input digital (Press, Release, Hold)
// ============================================================
class Button {
public:
    void setInvert(bool invert) { isInverted = invert; }
    void setDebounceMs(uint16_t ms) { debounceMs = ms; }

    void update(bool rawInput, uint32_t nowMs);

    bool isPressed();
    bool isReleased();
    bool isDown() const { return stableState.load(); }

private:
    static bool consumeOne(std::atomic<uint8_t>& counter);

    bool isInverted = false;
    uint16_t debounceMs = 20;

    bool lastRaw = false;
    uint32_t lastChangeMs = 0;

    std::atomic<bool> stableState{false};
    std::atomic<uint8_t> pressCount{0};
    std::atomic<uint8_t> releaseCount{0};
};

// ============================================================
// EncoderJoints - encoder analog, posisi, rotasi
// ============================================================
class EncoderJoints {
public:
    float analogInput = 0.0f;
    float rotationoffset = 0.0f;

    uint16_t analoginputmaxmin[2] = {0, 1023};
    float radmaxmin[2] = {-2.0f * M_PI, 2.0f * M_PI};

    Vector3 axis = Vector3(0.0f, 0.0f, 1.0f);
    Vector3 offsetPosition = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 localPosition = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 worldPosition = Vector3(0.0f, 0.0f, 0.0f);
    Quaternion worldRotation = Quaternion();

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

    float getDegreeOffset();
    float getRadOffset();
    float asRawRad();
    float asRawDegree();
    float asDegree();
    float asRad();
    Quaternion asQuat();

private:
    static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
};

// ============================================================
// InputChannels - buffer data input dari remote/sensor
// ============================================================
class InputChannels {
public:
    static const int SIZE = 7;

    void set(int index, float value);
    void set(const float* arr);

    float getChannel(int index);
    float* bindChannels();
    float* bindChannel(int index);

    void clear();
    bool isPressed(int index, float threshold = 0.5f);
    void applyDeadzone(int index, float deadzone);
    void applyDeadzoneAll(float deadzone);

private:
    float data[SIZE] = {0.0f};
};

// ============================================================
// KinematicChain - rantai kinematika serial
// ============================================================
class KinematicChain {
public:
    KinematicChain();

    int addJoint(const EncoderJoints& joint);
    int addJoint();
    int getJointCount();

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

    void setJointAxis(int index, const Vector3& axis);
    void setJointOffsetPosition(int index, const Vector3& pos);
    void setJointOffsetDegree(int index, float degreeOffset);
    void setJointOffsetRad(int index, float radOffset);
    void setJointMaxMin(int index, float minRad, float maxRad);
    void setJointAnalogMaxMin(int index, float minVal, float maxVal);

    void setAnalogInput(int index, float value);
    void setJointAngleRad(int index, float angleRad);
    void setJointAngleDegree(int index, float angleDeg);

    void compute();

    Quaternion getJointQuat(int index);
    Vector3 getJointPos(int index);
    EncoderJoints* getJoint(int index);
    float getJointAngleDegree(int index);
    float getJointAngleRad(int index);

private:
    bool valid(int index);

    std::vector<EncoderJoints> joints;
};

using KinematicArm = KinematicChain;

} // namespace stelle
