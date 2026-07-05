# Flight Computer — HPR Data Logger

A lightweight, standalone data-acquisition flight computer for high-power rocketry (HPR), built on an Arduino Nano. It logs barometric altitude, 3-axis acceleration, and 3-axis angular rate to an SD card at 25 Hz, then a companion Python script turns the raw log into a quick-look flight summary and plots.

This is a **passive logger, not a flight controller** — it does not fire ejection charges or actuate anything. It's meant to answer "what did the rocket actually do?" after the flight.

## Features

- **25 Hz sampling** on a drift-free absolute-tick scheduler (no `delay()`-based timing drift over a multi-minute flight)
- **Barometric altitude (AGL)** via BMP280, referenced to a ground-level pressure calibrated on the pad before launch
- **±16 g accelerometer / ±2000 °/s gyro** via MPU6050 — full-scale ranges chosen for motor burn and any tumble/spin recovery events
- **Gyro bias nulling** during ground calibration
- **Automatic incrementing log files** (`FLT_0000.CSV`, `FLT_0001.CSV`, …) — never overwrites a previous flight
- **Fail-fast fault handling**: any missing sensor, failed SD mount, or lost file handle mid-flight halts with a distinct audible/visual fault pattern rather than silently logging garbage
- **Unconditional flush-to-SD every row**: trades a small amount of write latency/SD wear for the guarantee that a mid-flight brownout or power loss costs at most one sample, not a buffered second of data
- **Post-flight analysis script** (`analysis/plot_flight.py`): altitude, acceleration magnitude, and gyro-rate plots, plus apogee altitude, peak acceleration, and estimated motor-burn duration

## Hardware

| Component | Role | Interface |
|---|---|---|
| Arduino Nano (ATmega328P) | Main controller | — |
| BMP280 | Barometric altimeter | I2C @ 0x76 |
| MPU6050 | 6-DOF accelerometer/gyro | I2C @ 0x68 (AD0 → GND) |
| SD card module | Log storage | SPI, CS → D10 |
| LED | Status indicator | D4, active HIGH |
| Buzzer | Audible event feedback | D5, active HIGH |

### Wiring

```
BMP280 / MPU6050   SDA → A4   |  SCL → A5   |  VCC → 3.3 V  |  GND → GND
SD module          MOSI → D11 |  MISO → D12 |  SCK → D13    |  CS → D10  |  VCC → 5 V  |  GND → GND
```

Both I2C sensors share the bus (SDA/SCL are common), so no additional wiring is needed to combine them.

## Firmware architecture

```
firmware/
├── flight_computer.ino   # setup()/loop() — scheduler and top-level flow
├── config.h              # pins, sample rate, sensor ranges, calibration settings
├── sensors.{h,cpp}       # BMP280 + MPU6050 init, calibration, per-sample read
├── logger.{h,cpp}        # SD mount, auto-numbered file creation, CSV writes
└── feedback.{h,cpp}      # LED/buzzer boot-OK, fault, and file-opened patterns
```

**Boot sequence:** sensor init → SD mount → ground calibration (pressure reference + gyro bias) → two beeps/LED-on = armed and logging.

**Fault behavior:** three long beeps + rapid LED blink, then halt — deliberately unrecoverable, since a rocket in flight has no way to receive a "retry" command.

## Requirements

**Firmware** (Arduino IDE):
- [Adafruit BMP280](https://github.com/adafruit/Adafruit_BMP280_Library) + Adafruit Unified Sensor (install when prompted)
- [MPU6050 by Electronic Cats](https://github.com/ElectronicCats/mpu6050)
- `SD` (built-in)

**Analysis** (Python 3.9+):
```bash
pip install -r analysis/requirements.txt
```

## Usage

1. Flash `firmware/flight_computer.ino` to the Nano (Arduino IDE or `arduino-cli`).
2. Before launch, power on with the rocket stationary and vertical on the pad — the ground-calibration step needs a stable reference.
3. Wait for two beeps + solid LED = armed.
4. Recover the rocket, pull the SD card, find the newest `FLT_NNNN.CSV`.
5. Run:
   ```bash
   python analysis/plot_flight.py FLT_0000.CSV
   ```
   This prints apogee altitude/time, peak acceleration, and estimated burn duration, and saves a 3-panel PNG plot alongside the CSV.

> **Before any real flight:** confirm `SERIAL_STREAM` is `0` in `config.h`. Leaving Serial streaming on costs ~2 ms per 40 ms sample loop and is intended for bench testing only.

## Design notes

- **Flush-on-every-write is intentional**, not an oversight — for a single-use flight log, losing at most one 40 ms sample to a power interruption is worth more than the write-latency/wear savings of buffering.
- **Full-scale ranges are hardcoded to match `config.h`** (`ACCEL_SCALE`/`GYRO_SCALE` in `sensors.cpp` are derived by hand from `ACCEL_FULL_SCALE`/`GYRO_FULL_SCALE`). If you change the full-scale range, update these two constants to match — they are not currently auto-derived from the macros.
- Fatal errors halt permanently by design; there is no in-flight recovery path, since a stuck-and-restarting logger is worse than a logger that stops cleanly.

## License

MIT — see [LICENSE](LICENSE). Contributions and forks welcome.
