# Architecture

## Runtime model

Stelle runs on the Arduino-ESP32 framework (FreeRTOS underneath, dual-core).
`setup()` wires up all singletons and launches five pinned tasks; `loop()` is empty.

| Task | Entry | Stack | Prio | Purpose |
|---|---|---|---|---|
| `SerialComA_Task` | `TaskSerialParse` | 4096 | 1 | Parse frame stream from `HardwareSerial(1)` (GPIO16/17) |
| `SerialUSB_Task` | `TaskSerialParse` | 4096 | 1 | Same parser bound to USB CDC `Serial` |
| `FKComputer_Task` | `TaskForwardKinematicsComputer` | 4096 | 1 | Propagate joint angles through `KinematicChain` |
| `SensorReader_Task` | `TaskSensorReader` | 4096 | 1 | Sample ADS1115s → `EncoderJoints.analogInput` |
| `PowerKeeper_Task` | `PowerKeeper` | 2048 | 1 | Drive power rail on GPIO27 |

## Data flow

```
analog pots ──▶ ADS1115 (I2C, Wire1) ──▶ SensorReader task
                                            │
                                            ▼
                                   EncoderJoints.analogInput
                                            │  map + offset
                                            ▼
                                       joint angle (rad)
                                            │
                                            ▼
                          KinematicChain ──▶ world position / quaternion
                                            │
             ┌──────────────────────────────┴──────────────┐
             ▼                                             ▼
     FKComputer publishes                        Communication task
                                                    │  Packet::pack()
                                                    ▼
                                          0xAA 0x55 | cmd | len | payload
```

Shared objects (`config`, `channels`, `Kinematics`, `log1`, `currentPowerState`)
are declared in `Stelle.ino` and `extern`-ed everywhere else.

## Synchronisation

`Mutexes.h` exposes one semaphore per shared resource:

- `mutexLog` — guards the logger
- `mutexQuaternionOffset`, `mutexVector3Offset`, `mutexIMUQuaternionOffset` — calibration state
- `mutexUseIMU` — IMU enable flag
- `mutexSaveConfig` — serialises NVS writes (`Preferences` is not re-entrant across tasks)

Rule of thumb used throughout: take the mutex, mutate, give immediately. NVS writes are
the only long operations and are always performed under `mutexSaveConfig`.

## Wire format

Frames are `0xAA 0x55` then `cmd:u8`, `len:u8`, payload.
Payloads are packed C structs:

```c
struct Vector3Data     { float x, y, z; };          // 12 B
struct QuaternionData  { float w, x, y, z; };       // 16 B
struct InputChannelsData { float ch[7]; };          // 28 B
```

`Vector3Packet`, `QuaternionPacket` and `InputChannelsPacket` wrap these and
`memcpy` between native types and the byte buffer. **Consequence:** the host side
must be little-endian with the same `float` size (4 B) — i.e. any ESP32/PC pair is fine,
but a different MCU or a big-endian host needs an explicit serialisation layer.

## Logger

`Logger` is a 256-byte ring buffer, line-oriented:

- writes always succeed (oldest complete line is discarded when full)
- `readUnread()` drains what the client has not seen yet
- `readAllHistory()` snapshots everything still buffered

That is what makes it safe to call from any task under a mutex without blocking the
sample loop: a slow or absent reader never stalls the firmware.

