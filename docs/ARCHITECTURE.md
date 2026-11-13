# Architecture

Stelle runs on ESP-IDF 5.x, but the code is written in an Arduino-shaped style:
global objects, one mutex per shared resource, and thin driver shims that
re-expose the Arduino API on top of IDF primitives. This document describes what
is actually in `main/`.

## Entry point

`main/setup.cpp` holds `app_main()`. It calls `setupSystem()` and then
`vTaskDelete(NULL)` — `app_main` does not idle, the pinned tasks do the work.

```
app_main()
  └─ setupSystem()
       1. create 13 mutexes
       2. log1.begin()             ring buffer
       3. initTaskPowerKeeper()    GPIO27 + PowerKeeper_Task
       4. Wire1.begin(25, 26)      I2C bus
       5. config.begin(&prefs)     load NVS config
       6. initTaskSerialComA()     HardwareSerial(1), GPIO16/17
       7. initTaskSerialUSB()      USB CDC `Serial`
       8. initTaskForwardKinematicsComputer()
       9. initTaskSensorReader()   ADS1115s + sampling task
```

Note that `initTaskPowerKeeper()` already calls `currentPowerState.begin()`, and
`setupSystem()` calls it once more before that. `PowerState::begin()` is
idempotent (`gpio_config` + `gpio_set_level`), so this only re-runs setup.

## Layout

```
main/
├── setup.cpp/.h            app_main, setupSystem, ALL globals + externs
├── Kconfig.projbuild       CONFIG_STELLE_*
├── CMakeLists.txt
├── core/
│   ├── GeoLib.h            Vector3 / Quaternion math (header-only)
│   ├── MechLib.h/.cpp      Button, EncoderJoints, InputChannels, KinematicChain
│   ├── Logger.h/.cpp       ring buffer + logI/logW/logE
│   └── Packets.h           wire structs, Cmd enum, payload-length table
├── drivers/
│   ├── ADS1115.h/.cpp      ADS1115 driver
│   ├── TwoWire.h/.cpp      Wire1 shim over i2c_master
│   ├── HardwareSerial.h/.cpp  HardwareSerial shim over UART / USB CDC
│   └── Preferences.h/.cpp  NVS shim over nvs_*
└── modules/
    ├── Communication.h/.cpp        frames, command dispatch, both parser tasks
    ├── ConfigManager.h/.cpp        NVS-backed configuration
    ├── ForwardKinematicsComputer.* 50 Hz FK task + the 6-joint table
    ├── SensorReader.h/.cpp         100 Hz ADS1115 sampling task
    └── PowerManager.h/.cpp         PowerState + rail keeper task
```

`setup.h` is the shared header: it `extern`-declares every global object and
mutex so no module has to own them.

## Tasks

| Task | Entry | Stack | Prio | Core | Period |
|---|---|---|---|---|---|
| `SerialComA_Task` | `TaskSerialParse` | 4096 | 1 | 1 | 10 ms poll |
| `SerialUSB_Task` | `TaskSerialParse` | 4096 | 1 | 1 | 10 ms poll |
| `FKComputer_Task` | `TaskForwardKinematicsComputer` | 4096 | 1 | 1 | 20 ms (50 Hz) |
| `SensorReader_Task` | `TaskSensorReader` | 4096 | 1 | 1 | 10 ms (100 Hz) |
| `PowerKeeper_Task` | `PowerKeeper` | 2048 | 1 | 1 | `CONFIG_STELLE_POWER_POLL_MS` |

Every task runs at priority 1 on core 1. There is no priority-based
pre-emption between them; they interleave on the tick and on blocking calls.

## Data flow

```
analog pots ──▶ ADS1115 (Wire1) ──▶ SensorReader_Task (100 Hz)
                                        │ mutexJoints
                                        ▼
                                Kinematics.setAnalogInput()
                                        │
                                        ▼
                    EncoderJoints: map analog→rad, + rotationoffset
                                        │
                                        ▼
                    FKComputer_Task (50 Hz): Kinematics.compute()
                                        │ mutexJoints
                                        ▼
                          worldPosition / worldRotation per joint
                                        │
                                        ▼
                    TaskSerialParse ──▶ sendFrame() ──▶ host
```

## Joint table

Built in `initTaskForwardKinematicsComputer()` with `addJointConfig(...)`.
Arguments are `(axis, offsetPosition, offsetDegree, minRad, maxRad, analogMin, analogMax)`.

| Joint | Role | Axis | Link offset (m) | Range | ADC window |
|---|---|---|---|---|---|
| 0 | Base yaw | Z | (0, 0, 0) | ±180° | 30 – 26170 |
| 1 | Shoulder pitch | Y | (0, 0, 0.25) | ±90° | 43 – 13010 |
| 2 | Elbow pitch | Y | (0, 0, 0.20) | ±120° | 0 – 32767 |
| 3 | Wrist pitch | Y | (0.15, 0, 0) | ±90° | 0 – 32767 |
| 4 | Wrist roll | X | (0.05, 0, 0) | ±180° | 0 – 32767 |
| 5 | Tool / gripper | Z | (0.02, 0, 0) | ±90° | 0 – 32767 |

Joints 2, 3, 4 and 5 use the full `0 – 32767` window, which is wider than the
ADC actually produces for a 3.3 V single-ended input at PGA ±4.096 V. Treat
those ranges as placeholders until each joint is calibrated on the bench.

## Shared state

All of it is declared in `setup.cpp` and `extern`-ed in `setup.h`:

| Object | Guarded by |
|---|---|
| `Kinematics` | `mutexJoints` |
| `channels` | `mutexChannels` |
| `log1` | `mutexLog` |
| `currentPowerState` | `mutexPowerState` |
| `config` (NVS writes) | `mutexSaveConfig` |
| `config` wifi/bt/offset/IMU fields | one mutex each, see below |

## Mutexes

13 semaphores, all created in `setupSystem()`:

`mutexPowerState`, `mutexJoints`, `mutexChannels`, `mutexLog`,
`mutexWifiSSID`, `mutexWifiPassword`, `mutexBtName`, `mutexBtPassword`,
`mutexUseIMU`, `mutexIMUQuaternionOffset`, `mutexQuaternionOffset`,
`mutexVector3Offset`, `mutexSaveConfig`.

Pattern used everywhere: `xSemaphoreTake(m, portMAX_DELAY)` → mutate →
`xSemaphoreGive(m)`. NVS writes always go through `mutexSaveConfig`, because
`Preferences`/NVS is not re-entrant across tasks.

`mutexLog` is taken by `logI`/`logW`/`logE` themselves, so callers of those
macros must not already hold it.

## Logger

Fixed ring buffer of `CONFIG_STELLE_RING_BYTES` (4096 by default),
line-oriented and non-blocking:

* when full, the oldest complete line is discarded so readers still see whole lines
* `readUnread()` drains what the client has not seen
* `readAllHistory()` snapshots everything still buffered

`Logger` has no internal lock — `mutexLog` is the caller's job
(`logI`/`logW`/`logE` take it for you).

## Adding a command

1. Add the opcode to `enum class Cmd` in `core/Packets.h`.
2. Add its inbound payload length to `PayloadLenForCommand()` — this is the
   only place the parser learns the frame length, since there is no length byte.
3. Handle it in `executeCommand()` in `modules/Communication.cpp`.
4. Add the reply encoding on the host side.