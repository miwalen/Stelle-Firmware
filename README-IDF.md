# Stelle - SUPER Teleoperator (ESP-IDF)

Branch `esp-idf`: port firmware Stelle dari Arduino-ESP32 ke **ESP-IDF 5.x**.

Branch `main` tetap berisi versi Arduino (folder `central/`). Branch ini
menambahkan build ESP-IDF di root + folder `main/`.

## Struktur

```
CMakeLists.txt          project ESP-IDF
sdkconfig.defaults      target esp32, 240 MHz, FreeRTOS 1000 Hz
partitions.csv          nvs + factory 3 MB
main/
  CMakeLists.txt        daftar source
  Kconfig.projbuild     CONFIG_STELLE_*
  Stelle.cpp/.h         pengganti Stelle.ino (setup/loop/app_main, objek global)
  TwoWire.cpp/.h        Wire1 - pengganti TwoWire Arduino (di atas i2c_master)
  HardwareSerial.cpp/.h HardwareSerial - pengganti HardwareSerial Arduino
  ADS1115.cpp/.h        driver ADC, API identik versi Arduino
  Communication.cpp/.h  protokol biner, parser, task UART + USB
  ConfigManager.cpp/.h  NVS: wifi, BT, useIMU, offset
  ForwardKinematicsComputer.cpp/.h  task FK 50 Hz + tabel 6 joint
  GeoLib.h              Vector3 / Quaternion (header-only)
  Logger.h/.cpp         ring buffer + logI/logW/logE
  MechLib.cpp/.h        Button, EncoderJoints, InputChannels, KinematicChain
  Mutexes.cpp/.h        13 mutex, sama seperti versi Arduino
  Packets.h             struct wire-format + enum Cmd + tabel panjang payload
  PowerManager.cpp/.h   PowerState + task PowerKeeper
  SensorReader.cpp/.h   task sampling ADS 100 Hz
docs-idf/               (kosong, siap diisi)
tools-idf/stelle_client.py  client pyserial
```

## Pemakaian I2C dan Serial

Ditulis seperti Arduino:

```cpp
Wire1.begin(25, 26);                              // SDA, SCL
ads1.begin(Wire1, ADS1115::ADDR_GND);

HardwareSerial SerialComA(1);
SerialComA.setRxBufferSize(256);
SerialComA.begin(115200, SERIAL_8N1, 16, 17);
```

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
`PayloadLenForCommand()` di `Packets.h`.

## Credits

Firmware oleh [Miwalen](https://github.com/Miwalen). MIT.
