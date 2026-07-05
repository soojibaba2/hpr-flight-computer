/*
 * flight_computer.ino
 *
 * Arduino Nano flight computer — Track 2 HPR data acquisition project
 *
 * Hardware:
 *   - Arduino Nano (ATmega328P)
 *   - BMP280   barometric altimeter          (I2C, addr 0x76)
 *   - MPU6050  6-DOF accelerometer/gyro      (I2C, addr 0x68)
 *   - SD card module                         (SPI, CS = D10)
 *   - LED      status indicator              (D4, active HIGH via 220 Ω)
 *   - Buzzer   event feedback                (D5, active HIGH)
 *
 * Wiring summary:
 *   BMP280/MPU6050  SDA → A4 | SCL → A5 | VCC → 3.3 V | GND → GND
 *   SD module   MOSI→D11 | MISO→D12 | SCK→D13 | CS→D10 | VCC→5V | GND→GND
 *   MPU6050  AD0 → GND (fixes I2C address at 0x68)
 *
 * Libraries (install via Arduino Library Manager):
 *   - Adafruit BMP280          (search "Adafruit BMP280")
 *   - Adafruit Unified Sensor  (dependency, install when prompted)
 *   - MPU6050 by Electronic Cats  (search "MPU6050")
 *   - SD                       (built-in)
 *
 * Behaviour:
 *   1. Boot  → sensors init → SD mount → ground calibration
 *   2. Two beeps + LED on = ready and logging
 *   3. Logs at 25 Hz:  timestamp_ms, altitude_m(AGL), ax/ay/az(m/s²), gx/gy/gz(°/s)
 *   4. New file per power cycle: FLT_0000.CSV, FLT_0001.CSV …
 *   5. Fatal error → three long beeps + rapid LED blink → halt
 *
 * Post-processing:
 *   See analysis/plot_flight.py for a Python quick-look script.
 */

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_BMP280.h>
#include <MPU6050.h>

#include "config.h"
#include "feedback.h"
#include "sensors.h"
#include "logger.h"

// ── next-tick timestamp (drift-free 25 Hz scheduler) ─────────────────────
static uint32_t nextTickMs = 0;

// ─────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  Serial.println(F("=== Flight computer v1.0 ==="));

  feedback_init();
  sensors_init();     // fatal halt if BMP280 or MPU6050 absent
  logger_init();      // fatal halt if SD card fails
  sensors_calibrate();

  feedback_boot_ok();
  Serial.println(F("Logging started."));

  nextTickMs = millis();
}

// ─────────────────────────────────────────────────────────────────────────
void loop() {
  uint32_t now = millis();
  if (now < nextTickMs) return;
  nextTickMs += LOOP_PERIOD_MS;

  SensorData data;
  sensors_read(&data);
  logger_write(&data);

#if SERIAL_STREAM
  // Pipe to Serial for bench monitoring (disable in flight to save ~2 ms/row)
  Serial.print(data.timestamp_ms); Serial.print(',');
  Serial.print(data.altitude_m, 2); Serial.print(',');
  Serial.print(data.ax, 3); Serial.print(',');
  Serial.print(data.ay, 3); Serial.print(',');
  Serial.print(data.az, 3); Serial.print(',');
  Serial.print(data.gx, 2); Serial.print(',');
  Serial.print(data.gy, 2); Serial.print(',');
  Serial.println(data.gz, 2);
#endif
}
