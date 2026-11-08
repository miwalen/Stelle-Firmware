# Stelle — SUPER Teleoperator

**S**telle is a **SUPER Teleoperator**: a teleoperation sensing rig that reports orientation as **position (x, y, z)** and **rotation (w, x, y, z)** quaternions derived from the **analog joint angles** read off each joint's analog encoder.

The firmware turns raw 10-bit ADC readings into calibrated radians, propagates them through a quaternion-based kinematic chain, and streams both the raw channels and the resolved pose over a binary serial protocol.

> **Status: work in progress.** This checkpoint has logging and encoders working end-to-end. IMU support, Wi-Fi/BT transport, and other subsystems are scaffolded but not complete — see [Roadmap](#roadmap).

---

## Features (current checkpoint)

| Subsystem | State |
|---|---|
| Line-oriented ring-buffer logger (fixed 256 B, non-blocking, task-safe) | ✅ working |
| Analog encoder → calibrated radians (ADC map + offset) | ✅ working |
| Forward kinematics chain (quaternion-based, per-joint axis + offset) | ✅ working |
| ADS1115 I²C ADC driver (2 units, second bus) | ✅ working |
| Config persistence via NVS (Wi-Fi, BT, offsets, `useIMU`) | ✅ working |
| Binary packet protocol over UART + USB CDC | ✅ working |
| FreeRTOS task layout (5 pinned tasks, mutex-guarded shared state) | ✅ working |
| IMU fusion (`useIMU` flag exists, driver not present) | 🚧 not yet |
| Wi-Fi / Bluetooth Classic transport | 🚧 not yet |
| Behaviour of this firmware as master vs. slave in the rig | 🚧 undecided |

## Hardware

- **MCU:** ESP32 (classic Bluetooth only — **not** BLE), 2 cores
- **I²C bus:** `Wire1`, SDA = GPIO25, SCL = GPIO26
- **ADC:** 2 × ADS1115 (16-bit, I²C)
- **Encoders:** analog potentiometer-type joints sampled through the ADS1115s
- **UART link:** `HardwareSerial(1)` @ 115200 8N1, RX = GPIO16, TX = GPIO17
- **USB link:** CDC `Serial` @ 115200
- **Power switch:** GPIO27

No schematic or wiring diagram yet. Board photos are pending.

## Build

1. Install the **ESP32 board package** in Arduino IDE (or `arduino-cli`).
2. Select the classic-ESP32 board variant (the one with Bluetooth Classic — **not** an S3/C3/BLE-only part).
3. Open `central/Stelle.ino` and upload.

```bash
# arduino-cli equivalent (board FQBN to be confirmed)
arduino-cli compile --fqbn esp32:esp32:esp32 central
arduino-cli upload  --fqbn esp32:esp32:esp32 -p <PORT> central
```

## Serial protocol

Framed as `0xAA 0x55 | cmd | len | payload…`.

| Opcode | Direction | Meaning |
|---|---|---|
| `0x01` | → device | Ping |
| `0x10` | → device | Set quaternion offset (persisted) |
| `0x11` | → device | Set vector3 offset (persisted) |
| `0x12` | → device | Set `useIMU` (persisted) |
| `0x13` | → device | Set Wi-Fi SSID (persisted) |
| `0x14` | → device | Set Wi-Fi password (persisted) |
| `0x15` | → device | Set BT name (persisted) |
| `0x16` | → device | Set BT password (persisted) |
| `0x17` | → device | Set IMU quaternion offset (persisted) |
| `0x20`–`0x23` | → device | Joint/secondary controls — *documented in source* |
| `0x30`–`0x31` | → device | Power / state controls — *documented in source* |
| `0x40`–`0x41` | → device | Query / status — *documented in source* |
| `0x50` | → device | Log fetch (read unread / read all history) |

Packet structs (`Vector3Data`, `QuaternionData`, `InputChannelsData`) are `memcpy`-ed verbatim, so **both ends must share endianness and struct packing** — see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

## Repository layout

```
Stelle-Firmware/
├── central/                   ESP32 firmware — central (teleoperator) side
│   ├── Stelle.ino             entry point, object wiring, task launch
│   ├── Communication.cpp/.h   binary protocol, UART + USB CDC tasks
│   ├── ConfigManager.cpp/.h   NVS-backed configuration
│   ├── MechLib.cpp/.h         Button, EncoderJoints, InputChannels, KinematicChain
│   ├── GeoLib.h               Vector3 / Quaternion math
│   ├── ForwardKinematicsComputer.*  FK task
│   ├── SensorReader.*         ADS1115 sampling task
│   ├── PowerManager.*         power-rail keeper task
│   ├── Logger.h               fixed ring-buffer logger
│   ├── Mutexes.cpp/.h         shared-state semaphores
│   ├── Packets.h              wire-format structs and packers
│   └── ADS1115.cpp/.h         ADC driver
├── docs/                      architecture, hardware, protocol notes
├── README.md
└── LICENSE
```

Planned: `peripheral/` firmware for the downstream board, sharing a `common/`
protocol layer with `central/`.

## Roadmap

- [ ] Add IMU (source/model TBD) and fuse with encoder-derived pose
- [ ] Decide and document the master/slave role of this firmware
- [ ] Wi-Fi and Bluetooth Classic transport layers
- [ ] Wiring diagram / board photos
- [ ] Closed-loop calibration tooling

## Credits

**Stelle** — SUPER Teleoperator. Firmware by [Miwalen](https://github.com/Miwalen).

## License

[MIT](LICENSE)

