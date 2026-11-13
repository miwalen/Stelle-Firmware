# Stelle — SUPER Teleoperator (ESP-IDF)

**S**telle is a **SUPER Teleoperator**: a teleoperation sensing rig that reports
joint orientation as **position (x, y, z)** and **rotation (w, x, y, z)**
quaternions, derived from the **analog joint angles** read off each joint's
analog encoder.

The firmware turns raw ADS1115 readings into calibrated radians, propagates
them through a quaternion kinematic chain, and streams both the raw channels
and the resolved pose over a binary serial protocol.

This is branch `esp-idf`: the port from Arduino-ESP32 to **ESP-IDF 5.x**. The
Arduino version lives on `main`.

> **Status: work in progress.** Logging and encoders work end-to-end.
> IMU support and Wi-Fi/BT transport are *not* implemented — see
> [Roadmap](#roadmap). Everything below is checked against the source in
> `main/`.

---

## Features (current checkpoint)

| Subsystem | State |
|---|---|
| Line-oriented ring-buffer logger (`CONFIG_STELLE_RING_BYTES`, default 256 B, non-blocking) | ✅ working |
| Analog encoder → calibrated radians (ADC map + offset) | ✅ working |
| Forward kinematics chain (quaternion, per-joint axis + offset) | ✅ working |
| ADS1115 I²C ADC driver (2 units on `Wire1`) | ✅ working |
| Config persistence via NVS (Wi-Fi strings, BT strings, offsets, `useIMU`) | ✅ working |
| Binary packet protocol over UART1 + USB CDC | ✅ working |
| FreeRTOS task layout (5 pinned tasks, mutex-guarded shared state) | ✅ working |
| IMU fusion | 🚧 flag persists, no driver |
| Wi-Fi / Bluetooth Classic transport | 🚧 strings persist, no stack wired |
| Master/slave role of this firmware | 🚧 undecided |

## Hardware

- **MCU:** ESP32 (classic ESP32-WROOM/WROVER — dual core, 240 MHz)
- **I²C bus:** `Wire1`, SDA = GPIO25, SCL = GPIO26
- **ADC:** 2 × ADS1115 — `0x48` (`ADDR_GND`) and `0x4A` (`ADDR_SDA`)
- **Encoders:** analog potentiometer-type joints sampled through the ADS1115s
- **UART link:** `HardwareSerial(1)` @ 115200 8N1, RX = GPIO16, TX = GPIO17
- **USB link:** CDC `Serial` (`HardwareSerial(0)`) @ 115200
- **Power rail:** GPIO27

`Wire1` and `HardwareSerial` are thin shims in `main/drivers/`, not ESP-IDF
APIs — they re-expose the Arduino surface on top of `i2c_master` and
`uart_*`/USB-CDC.

No schematic or wiring diagram yet. Board photos pending.

## Build

Requires ESP-IDF **v5.2 or newer** (`i2c_master`).

```bash
. $IDF_PATH/export.sh        # Windows: %IDF_PATH%\export.bat
idf.py set-target esp32
idf.py build
idf.py -p COM3 flash monitor
```

`CONFIG_STELLE_POWER_POLL_MS` (default 50) and `CONFIG_STELLE_RING_BYTES`
(default 4096) live under `idf.py menuconfig` → **Stelle firmware**.

## Serial protocol

```
0xAA 0x55 | cmd:u8 | payload[PayloadLenForCommand(cmd)] | checksum:u8
```

**There is no length byte on the wire.** The receiver derives the payload size
from the command table in `main/core/Packets.h`, and `checksum` is the XOR of
the command byte over every payload byte.

| Opcode | Direction | Meaning |
|---|---|---|
| `0x01` | → device | Ping (no reply) |
| `0x10` | → device | Set quaternion offset (persisted) |
| `0x11` | → device | Set vector3 offset (persisted) |
| `0x12` | → device | Set `useIMU` (persisted) |
| `0x13` | → device | Set Wi-Fi SSID (persisted) |
| `0x14` | → device | Set Wi-Fi password (persisted) |
| `0x15` | → device | Set BT name (persisted) |
| `0x16` | → device | Set BT password (persisted) |
| `0x17` | → device | Set IMU quaternion offset (persisted) |
| `0x20` | → device | Get joint position → 12 B `Vector3Data` |
| `0x21` | → device | Get joint quaternion → 16 B `QuaternionData` |
| `0x22` | → device | Get joint angle, degrees → 4 B float |
| `0x23` | → device | Get joint angle, radians → 4 B float |
| `0x24` | → device | Get joint count → 1 B `u8` |
| `0x30` | → device | Set input channels (28 B) |
| `0x31` | → device | Get input channels → 28 B |
| `0x40` | → device | Read unread log → `0x99` + text (max 255 B) |
| `0x41` | → device | Read full log history → `0x99` + text (max 255 B) |
| `0x50` | → device | Power off (rail drops, no reply) |

Two things bite integrators:

1. **Setters do not acknowledge.** `0x10`–`0x17`, `0x30` and `0x50` return
   nothing on the wire — they only log. A host waiting for an ack will hang.
2. **Log replies are a single frame capped at 255 B**, not chunked, and not
   guaranteed to end on a line boundary.

Packet structs (`Vector3Data` 12 B, `QuaternionData` 16 B, `InputChannelsData`
28 B) are `memcpy`-ed verbatim, so both ends must share endianness and `float`
size. Full reference: [docs/PROTOCOL.md](docs/PROTOCOL.md).

## Repository layout

```
StelleESPIDF/
├── CMakeLists.txt
├── sdkconfig.defaults
├── partitions.csv
├── README.md              this file
├── README-IDF.md          port notes (branch esp-idf)
├── CHANGELOG.md
├── main/
│   ├── CMakeLists.txt
│   ├── Kconfig.projbuild  CONFIG_STELLE_*
│   ├── setup.cpp/.h       app_main, setupSystem, all globals + externs
│   ├── core/              GeoLib, MechLib, Logger, Packets
│   ├── drivers/           ADS1115, TwoWire, HardwareSerial, Preferences
│   └── modules/           Communication, ConfigManager,
│                          ForwardKinematicsComputer, SensorReader, PowerManager
├── docs/
│   ├── ARCHITECTURE.md    runtime model, tasks, mutexes, joint table
│   ├── HARDWARE.md        pin map, ADC addresses, calibration windows
│   └── PROTOCOL.md        frame format, opcodes, parser behaviour
└── LICENSE
```

## Tasks

Five tasks, all priority 1 on core 1 — see
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md#tasks).

| Task | Period | Purpose |
|---|---|---|
| `SerialComA_Task` | 10 ms poll | frame parser on UART1 (GPIO16/17) |
| `SerialUSB_Task` | 10 ms poll | same parser on USB CDC |
| `FKComputer_Task` | 20 ms (50 Hz) | `Kinematics.compute()` |
| `SensorReader_Task` | 10 ms (100 Hz) | read both ADS1115s into the chain |
| `PowerKeeper_Task` | `CONFIG_STELLE_POWER_POLL_MS` | drive GPIO27 |

## Calibration status

Joints 0 and 1 have measured ADC windows (`30–26170` and `43–13010`).
Joints 2–5 still hold the placeholder `0–32767`, which is wider than the ADC
can physically produce — those joints will read compressed until calibrated.
Details in [docs/HARDWARE.md](docs/HARDWARE.md#joint-calibration-windows).

## Roadmap

- [ ] Add IMU (source/model TBD) and fuse with encoder-derived pose
- [ ] Decide and document master/slave role of this firmware
- [ ] Wi-Fi and Bluetooth Classic transport layers
- [ ] Measure real ADC windows for joints 2–5
- [ ] Make setters acknowledge, or document fire-and-forget as final
- [ ] Wiring diagram / board photos
- [ ] Closed-loop calibration tooling

## Credits

**Stelle** — SUPER Teleoperator. Firmware by [Miwalen](https://github.com/Miwalen).

## License

[MIT](LICENSE)