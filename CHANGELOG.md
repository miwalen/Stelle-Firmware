# Changelog

All notable changes to this project are documented here.
Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/);
versioning follows [Semantic Versioning](https://semver.org/).

## [0.1.0] — 2026-11-09

First tracked checkpoint: **logging and encoders working**.

### Added
- Line-oriented ring-buffer `Logger` with unread/history cursors.
- `EncoderJoints`: analog ADC reading → calibrated radians with per-joint offset.
- `KinematicChain`: quaternion forward-kinematics across joint chain.
- ADS1115 driver for two I²C units on `Wire1` (SDA 25 / SCL 26).
- Binary serial protocol (`0xAA 0x55`) over UART (GPIO16/17) and USB CDC.
- NVS-backed `ConfigManager`: Wi-Fi, BT, `useIMU`, quaternion/vector offsets.
- FreeRTOS task set: SerialComA, SerialUSB, FK computer, sensor reader, power keeper.
- Mutex-protected shared state (`Mutexes.h`).

### Known limitations
- IMU driver absent; `useIMU` is a persisted flag only.
- Wi-Fi / Bluetooth Classic transport not implemented.
- Master/slave role of this firmware not yet decided.
- No wiring diagram.

