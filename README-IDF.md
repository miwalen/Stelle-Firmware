# Stelle - SUPER Teleoperator (ESP-IDF)

Branch `esp-idf`: port firmware Stelle dari Arduino-ESP32 ke **ESP-IDF 5.x**.

Branch `main` tetap berisi versi Arduino. Branch ini memindahkan source ke
`main/` dengan struktur `core/` + `drivers/` + `modules/`, tapi gaya kodenya
sengaja dipertahankan mirip Arduino: objek global, satu mutex per resource,
dan shim tipis (`Wire`, `HardwareSerial`, `Preferences`) di atas API ESP-IDF.

## Struktur

```
CMakeLists.txt          project ESP-IDF
sdkconfig.defaults      target esp32, 240 MHz, FreeRTOS 1000 Hz
partitions.csv          nvs + factory 3 MB
main/
  CMakeLists.txt        daftar source
  Kconfig.projbuild     CONFIG_STELLE_*
  setup.cpp/.h          app_main + setupSystem + semua objek & mutex global
  core/
    GeoLib.h            Vector3 / Quaternion (header-only)
    MechLib.h/.cpp      Button, EncoderJoints, InputChannels, KinematicChain
    Logger.h/.cpp       ring buffer + logI/logW/logE
    Packets.h           struct wire-format + enum Cmd + tabel panjang payload
  drivers/
    ADS1115.cpp/.h      driver ADC, API identik versi Arduino
    TwoWire.cpp/.h      Wire1 di atas i2c_master
    HardwareSerial.cpp/.h  HardwareSerial di atas UART / USB CDC
    Preferences.cpp/.h  NVS shim di atas nvs_*
  modules/
    Communication.cpp/.h        sendFrame, executeCommand, TaskSerialParse
    ConfigManager.cpp/.h        NVS: wifi, BT, useIMU, offset
    ForwardKinematicsComputer.* task FK 50 Hz + tabel 6 joint
    SensorReader.cpp/.h         task sampling ADS 100 Hz
    PowerManager.cpp/.h         PowerState + task PowerKeeper
docs/
  ARCHITECTURE.md       model runtime, task, mutex, tabel joint
  HARDWARE.md           pin map, alamat ADS1115, window kalibrasi
  PROTOCOL.md           format frame, opcode, perilaku parser
```

## Pemakaian I2C dan Serial

Ditulis seperti Arduino:

```cpp
Wire1.begin(25, 26);                              // SDA, SCL
ads1.begin(Wire1, ADS1115::ADDR_GND);             // 0x48
ads2.begin(Wire1, ADS1115::ADDR_SDA);             // 0x4A

HardwareSerial SerialComA(1);
SerialComA.setRxBufferSize(256);
SerialComA.begin(115200, SERIAL_8N1, 16, 17);
```

## Task

Lima task, semuanya priority 1 di core 1:

| Task | Periode | Fungsi |
|---|---|---|
| `SerialComA_Task` | poll 10 ms | parser frame di UART1 (GPIO16/17) |
| `SerialUSB_Task` | poll 10 ms | parser frame yang sama di USB CDC |
| `FKComputer_Task` | 20 ms (50 Hz) | `Kinematics.compute()` |
| `SensorReader_Task` | 10 ms (100 Hz) | baca 2x ADS1115, tulis ke chain |
| `PowerKeeper_Task` | `CONFIG_STELLE_POWER_POLL_MS` | tulis GPIO27 |

## Build

Butuh ESP-IDF **v5.2 atau lebih baru** (`i2c_master`).

```bash
. $IDF_PATH/export.sh        # Windows: %IDF_PATH%\export.bat
idf.py set-target esp32
idf.py build
idf.py -p COM3 flash monitor
```

## Protokol

```
0xAA 0x55 | cmd | payload[len(cmd)] | xor
```

Tidak ada length byte di wire: panjang payload ditentukan tabel
`PayloadLenForCommand()` di `Packets.h`. Checksum = XOR dari byte cmd dan
seluruh byte payload.

Detail lengkap (opcode, reply, perilaku parser) ada di
[docs/PROTOCOL.md](docs/PROTOCOL.md).

## Credits

Firmware oleh [Miwalen](https://github.com/Miwalen). MIT.