#include "MechLib.h"

// ============================================================
// Button
// ============================================================

void Button::setInvert(bool invert)     { isInverted = invert; }
void Button::setDebounceMs(uint16_t ms) { debounceMs = ms; }
bool Button::isDown() const             { return stableState.load(); }

void Button::update(bool rawInput, uint32_t nowMs) {
    bool raw = (rawInput != isInverted);

    // Bacaan berubah -> reset timer debounce
    if (raw != lastRaw) {
        lastRaw = raw;
        lastChangeMs = nowMs;
    }

    // Bacaan stabil cukup lama dan beda dari status resmi -> terjadi event
    if (lastRaw != stableState.load() && (nowMs - lastChangeMs) >= debounceMs) {
        stableState.store(lastRaw);

        if (lastRaw) {
            if (pressCount.load() < 255) pressCount.fetch_add(1);     // event press
        } else {
            if (releaseCount.load() < 255) releaseCount.fetch_add(1); // event release
        }
    }
}

// Ambil satu event dari counter secara atomik
bool Button::consumeOne(std::atomic<uint8_t>& counter) {
    uint8_t c = counter.load();
    while (c > 0) {
        if (counter.compare_exchange_weak(c, c - 1)) return true;
    }
    return false;
}

bool Button::isPressed()  { return consumeOne(pressCount); }
bool Button::isReleased() { return consumeOne(releaseCount); }

// ============================================================
// EncoderJoints
// ============================================================
float EncoderJoints::mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void EncoderJoints::setAnalogInput(float value) {
    analogInput = value;
}

void EncoderJoints::setAngleRad(float angleRad) {
    float raw = angleRad - rotationoffset;

    // Wrap ke rentang -PI .. PI
    while (raw > M_PI)  raw -= (2.0f * M_PI);
    while (raw < -M_PI) raw += (2.0f * M_PI);

    analogInput = mapFloat(raw, radmaxmin[0], radmaxmin[1],
                           analoginputmaxmin[0], analoginputmaxmin[1]);
}

void EncoderJoints::setAngleDegree(float angleDeg) {
    setAngleRad(angleDeg * (M_PI / 180.0f));
}

void EncoderJoints::setMaxMin(float minRad, float maxRad) {
    radmaxmin[0] = minRad;
    radmaxmin[1] = maxRad;
}

void EncoderJoints::setAnalogMaxMin(float minVal, float maxVal) {
    analoginputmaxmin[0] = minVal;
    analoginputmaxmin[1] = maxVal;
}

void EncoderJoints::setOffsetDegree(float degreeOffset) {
    rotationoffset = degreeOffset * (M_PI / 180.0f);
}

void EncoderJoints::setOffsetRad(float radOffset) {
    rotationoffset = radOffset;
}

void EncoderJoints::setAxis(const Vector3& a) {
    axis = a;
}

void EncoderJoints::setAxis(float x, float y, float z) {
    axis = Vector3(x, y, z);
}

void EncoderJoints::setOffsetPosition(const Vector3& pos) {
    offsetPosition = pos;
}

void EncoderJoints::setOffsetPosition(float x, float y, float z) {
    offsetPosition = Vector3(x, y, z);
}

float EncoderJoints::getDegreeOffset() {
    return rotationoffset * (180.0f / M_PI);
}

float EncoderJoints::getRadOffset() {
    return rotationoffset;
}

float EncoderJoints::asRawRad() {
    return mapFloat(analogInput, analoginputmaxmin[0], analoginputmaxmin[1],
                    radmaxmin[0], radmaxmin[1]);
}

float EncoderJoints::asRawDegree() {
    float minDeg = radmaxmin[0] * (180.0f / M_PI);
    float maxDeg = radmaxmin[1] * (180.0f / M_PI);
    return mapFloat(analogInput, analoginputmaxmin[0], analoginputmaxmin[1], minDeg, maxDeg);
}

float EncoderJoints::asDegree() {
    float a = asRawDegree() + getDegreeOffset();
    while (a > 180.0f)  a -= 360.0f;
    while (a < -180.0f) a += 360.0f;
    return a;
}

float EncoderJoints::asRad() {
    float a = asRawRad() + rotationoffset;
    while (a > M_PI)  a -= (2.0f * M_PI);
    while (a < -M_PI) a += (2.0f * M_PI);
    return a;
}

Quaternion EncoderJoints::asQuat() {
    return Quaternion::FromAxisAngle(axis, roundf(asRad() * 100.0f) / 100.0f);
}

// ============================================================
// InputChannels
// ============================================================
void InputChannels::set(int index, float value) {
    if (index >= 0 && index < SIZE) {
        data[index] = value;
    }
}

void InputChannels::set(float v0, float v1, float v2, float v3, float v4, float v5, float v6) {
    data[0] = v0; data[1] = v1; data[2] = v2; data[3] = v3;
    data[4] = v4; data[5] = v5; data[6] = v6;
}

void InputChannels::set(const float* arr) {
    for (int i = 0; i < SIZE; i++) {
        data[i] = arr[i];
    }
}

float InputChannels::getChannel(int index) {
    if (index >= 0 && index < SIZE) return data[index];
    return 0.0f;
}

float* InputChannels::bindChannels() {
    return data;
}

float* InputChannels::bindChannel(int index) {
    if (index >= 0 && index < SIZE) return &data[index];
    return nullptr;
}

void InputChannels::clear() {
    for (int i = 0; i < SIZE; i++) data[i] = 0.0f;
}

bool InputChannels::isPressed(int index, float threshold) {
    if (index >= 0 && index < SIZE) return data[index] >= threshold;
    return false;
}

void InputChannels::applyDeadzone(int index, float deadzone) {
    if (index >= 0 && index < SIZE) {
        if (fabsf(data[index]) < deadzone) data[index] = 0.0f;
    }
}

void InputChannels::applyDeadzoneAll(float deadzone) {
    for (int i = 0; i < SIZE; i++) {
        if (fabsf(data[i]) < deadzone) data[i] = 0.0f;
    }
}

// ============================================================
// KinematicChain
// ============================================================
KinematicChain::KinematicChain() {
    joints.reserve(6);
}

bool KinematicChain::valid(int index) {
    return index >= 0 && index < (int)joints.size();
}

// --- Joint management ---
int KinematicChain::addJoint(const EncoderJoints& joint) {
    joints.push_back(joint);
    return (int)joints.size() - 1;
}

int KinematicChain::addJoint() {
    return addJoint(EncoderJoints());
}

int KinematicChain::getJointCount() {
    return (int)joints.size();
}

// --- Config satu baris ---
int KinematicChain::addJointConfig(const Vector3& axis, const Vector3& offsetPos,
                                   float offsetDegree, float minRad, float maxRad,
                                   float analogMin, float analogMax) {
    EncoderJoints j;
    j.setAxis(axis);
    j.setOffsetPosition(offsetPos);
    j.setOffsetDegree(offsetDegree);
    j.setMaxMin(minRad, maxRad);
    j.setAnalogMaxMin(analogMin, analogMax);
    return addJoint(j);
}

int KinematicChain::addJointConfig(float ax, float ay, float az,
                                   float px, float py, float pz,
                                   float offsetDegree, float minRad, float maxRad,
                                   float analogMin, float analogMax) {
    return addJointConfig(Vector3(ax, ay, az), Vector3(px, py, pz),
                          offsetDegree, minRad, maxRad, analogMin, analogMax);
}

void KinematicChain::setJointConfig(int index, const Vector3& axis, const Vector3& offsetPos,
                                    float offsetDegree, float minRad, float maxRad,
                                    float analogMin, float analogMax) {
    if (!valid(index)) return;
    joints[index].setAxis(axis);
    joints[index].setOffsetPosition(offsetPos);
    joints[index].setOffsetDegree(offsetDegree);
    joints[index].setMaxMin(minRad, maxRad);
    joints[index].setAnalogMaxMin(analogMin, analogMax);
}

void KinematicChain::setJointConfig(int index,
                                    float ax, float ay, float az,
                                    float px, float py, float pz,
                                    float offsetDegree, float minRad, float maxRad,
                                    float analogMin, float analogMax) {
    setJointConfig(index, Vector3(ax, ay, az), Vector3(px, py, pz),
                   offsetDegree, minRad, maxRad, analogMin, analogMax);
}

// --- Config per joint ---
void KinematicChain::setJointAxis(int index, const Vector3& axis) {
    if (valid(index)) joints[index].setAxis(axis);
}

void KinematicChain::setJointAxis(int index, float x, float y, float z) {
    if (valid(index)) joints[index].setAxis(x, y, z);
}

void KinematicChain::setJointOffsetPosition(int index, const Vector3& pos) {
    if (valid(index)) joints[index].setOffsetPosition(pos);
}

void KinematicChain::setJointOffsetPosition(int index, float x, float y, float z) {
    if (valid(index)) joints[index].setOffsetPosition(x, y, z);
}

void KinematicChain::setJointOffsetDegree(int index, float degreeOffset) {
    if (valid(index)) joints[index].setOffsetDegree(degreeOffset);
}

void KinematicChain::setJointOffsetRad(int index, float radOffset) {
    if (valid(index)) joints[index].setOffsetRad(radOffset);
}

void KinematicChain::setJointMaxMin(int index, float minRad, float maxRad) {
    if (valid(index)) joints[index].setMaxMin(minRad, maxRad);
}

void KinematicChain::setJointAnalogMaxMin(int index, float minVal, float maxVal) {
    if (valid(index)) joints[index].setAnalogMaxMin(minVal, maxVal);
}

// --- Input ---
void KinematicChain::setAnalogInput(int index, float value) {
    if (valid(index)) joints[index].setAnalogInput(value);
}

void KinematicChain::setJointAngleRad(int index, float angleRad) {
    if (valid(index)) joints[index].setAngleRad(angleRad);
}

void KinematicChain::setJointAngleDegree(int index, float angleDeg) {
    if (valid(index)) joints[index].setAngleDegree(angleDeg);
}

// --- Compute ---
void KinematicChain::compute() {
    for (size_t i = 0; i < joints.size(); i++) {
        if (i == 0) {
            // Joint pertama = base
            joints[0].worldRotation = Quaternion::Identity();
            joints[0].localPosition = vecRotate(joints[0].worldRotation, joints[0].offsetPosition);
            joints[0].worldPosition = joints[0].localPosition;
            continue;
        }

        EncoderJoints& cur  = joints[i];
        EncoderJoints& prev = joints[i - 1];

        // worldRotation[i] = worldRotation[i-1] * asQuat[i-1]
        cur.worldRotation = quatMultiply(prev.worldRotation, prev.asQuat());

        // localPosition[i] = offsetPosition[i] dirotasi oleh worldRotation[i]
        cur.localPosition = vecRotate(cur.worldRotation, cur.offsetPosition);

        // worldPosition[i] = localPosition[i] + worldPosition[i-1]
        cur.worldPosition = vecAdd(cur.localPosition, prev.worldPosition);
    }
}

// --- Getter ---
Quaternion KinematicChain::getJointQuat(int index) {
    if (valid(index)) return joints[index].worldRotation;
    return Quaternion::Identity();
}

Vector3 KinematicChain::getJointPos(int index) {
    if (valid(index)) return joints[index].worldPosition;
    return Vector3(0.0f, 0.0f, 0.0f);
}

EncoderJoints* KinematicChain::getJoint(int index) {
    return valid(index) ? &joints[index] : nullptr;
}

float KinematicChain::getJointAngleDegree(int index) {
    if (valid(index)) {
        return roundf(joints[index].asDegree());
    }
    return 0.0f; 
}

float KinematicChain::getJointAngleRad(int index) {
    if (valid(index)) {
        return roundf(joints[index].asRad() * 100.0f) / 100.0f;
    }
    return 0.0f;
}