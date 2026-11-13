# Hardware notes

Everything here is read out of the source. Where the source is ambiguous the
note says so instead of guessing.

## Pin map

| Signal | GPIO | Source of truth |
|---|---|---|
| I²C SDA | 25 | `Wire1.begin(25, 26)` in `setup.cpp` |
| I²C SCL | 26 | same call |
| UART RX | 16 | `SerialComA.begin(115200, SERIAL_8N1, 16, 17)` |
| UART TX | 17 | same call |
| Power rail | 27 | `PowerState::PIN` in `PowerManager.h` |
| USB | USB-CDC | `Serial` (`HardwareSerial(0)` re-bound to native USB) |

`Wire1` is not a second hardware I²C peripheral — it is the `TwoWire` shim in
`drivers/TwoWire.cpp` running on top of `i2c_master`.

## I²C devices

| Device | Address | Constant | Bus |
|---|---|---|---|
| ADS1115 #1 | **0x48** | `ADS1115::ADDR_GND` | `Wire1` |
| ADS1115 #2 | **0x4A** | `ADS1115::ADDR_SDA` | `Wire1` |

Both are constructed in `modules/SensorReader.cpp`:

```cpp
ads1.begin(Wire1, ADS1115::ADDR_GND);   // 0x48, ADDR -> GND
ads2.begin(Wire1, ADS1115::ADDR_SDA);   // 0x4A, ADDR -> SDA
```

So ADS1115 #2 needs its ADDR pin strapped to **SDA**, not SCL and not VDD.
`ADDR_SCL` (0x4B) and `ADDR_VDD` (0x49) exist in the enum but are unused.

Default analog front end: PGA ±4.096 V, 860 SPS. Both set in the driver's
constructor defaults; neither is changed at runtime.

## ADC channels

Each ADS1115 is read single-ended on AIN0/AIN1/AIN2 (3 channels each, 6 total).
The mapping into the kinematic chain is positional:

| Joint | Source |
|---|---|
| 0 | `ads1` AIN0 |
| 1 | `ads1` AIN1 |
| 2 | `ads1` AIN2 |
| 3 | `ads2` AIN0 |
| 4 | `ads2` AIN1 |
| 5 | `ads2` AIN2 |

From `TaskSensorReader`: `Kinematics.setAnalogInput(ch, raw[0][ch])` and
`Kinematics.setAnalogInput(ch + 3, raw[1][ch])`. AIN3 is not read on either unit.

## Joint calibration windows

From the `addJointConfig(...)` calls in `ForwardKinematicsComputer.cpp`:

| Joint | Axis | Link offset (m) | Range | analogMin | analogMax |
|---|---|---|---|---|---|
| 0 | Z | (0, 0, 0) | ±180° | 30 | 26170 |
| 1 | Y | (0, 0, 0.25) | ±90° | 43 | 13010 |
| 2 | Y | (0, 0, 0.20) | ±120° | 0 | 32767 |
| 3 | Y | (0.15, 0, 0) | ±90° | 0 | 32767 |
| 4 | X | (0.05, 0, 0) | ±180° | 0 | 32767 |
| 5 | Z | (0.02, 0, 0) | ±90° | 0 | 32767 |

**Joints 2–5 still use the placeholder window `0 – 32767`.** Only joints 0 and 1
carry measured limits. A linear map across the full 16-bit span will compress
the real mechanical travel into a fraction of the reported range, so readings
for those joints will look sluggish until the window is measured on the bench
and the values in `ForwardKinematicsComputer.cpp` are replaced.

## Power rail

GPIO27 is an output, configured in `PowerState::begin()` and driven by
`PowerKeeper_Task`. The task wakes every `CONFIG_STELLE_POWER_POLL_MS`
(default 50 ms) and writes the pin unconditionally — it does not skip writes
when the state is unchanged.

`PowerOff` (opcode `0x50`) flips `currentPowerState` to false; the keeper
drops the pin on its next pass. Worst-case latency is one poll interval.

## Encoders

Analog potentiometer-type joints, read through the ADS1115s. Per-joint mapping
lives in `EncoderJoints`: `analoginputmaxmin[2]` → `radmaxmin[2]`, plus a
`rotationoffset` in radians. All three are settable at runtime.

## TODO

- [ ] Board photo
- [ ] Wiring diagram
- [ ] Encoder model and usable mechanical travel
- [ ] Measure real `analogMin`/`analogMax` for joints 2, 3, 4 and 5
- [ ] Confirm the ADS1115 #2 ADDR strap is on SDA