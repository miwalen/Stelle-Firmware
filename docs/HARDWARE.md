# Hardware notes

> Skeleton. Fill in once the schematic / board photos exist.

## Pin map (from source)

| Signal | GPIO | Notes |
|---|---|---|
| I²C SDA | 25 | `Wire1` |
| I²C SCL | 26 | `Wire1` |
| UART RX | 16 | `SerialComA` (`HardwareSerial(1)`) |
| UART TX | 17 | `SerialComA` |
| Power rail | 27 | `PowerState`, driven by `PowerKeeper_Task` |

## I²C devices

- `ADS1115 ads1`
- `ADS1115 ads2`
(Addresses from each instance's constructor default / `setPGA` config — record them here.)

## Encoders

Analog potentiometer-type joints read through the ADS1115s.
Per-joint calibration is `{analog min/max} → {radian min/max}` plus `rotationoffset`,
all settable at runtime via `EncoderJoints::setMaxMin` / `setAnalogMaxMin` / `setOffsetRad`.

## TODO

- [ ] Board photo
- [ ] Wiring diagram / Fritzing export
- [ ] ADS1115 addresses (both units)
- [ ] Encoder model / range

