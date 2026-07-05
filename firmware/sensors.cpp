#include "sensors.h"
#include "config.h"
#include "feedback.h"
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <MPU6050.h>

// ── Module-private state ──────────────────────────────────────────────────

static Adafruit_BMP280 bmp;
static MPU6050         mpu(MPU6050_I2C_ADDR);

// Ground-level pressure in hPa; set during calibration
static float groundPressure_hPa = 1013.25f;

// Scale factors: raw int16 → m/s² and °/s
// These depend on the chosen full-scale ranges (see config.h).
// ±16 g  → sensitivity = 2048 LSB/g  → 1 LSB = 9.80665 / 2048  m/s²
// ±2000°/s → sensitivity = 16.4 LSB/(°/s) → 1 LSB = 1/16.4 °/s
static constexpr float ACCEL_SCALE = 9.80665f / 2048.0f;  // for ±16 g
static constexpr float GYRO_SCALE  = 1.0f    / 16.4f;     // for ±2000 °/s
static float gyroBiasX = 0.0f;
static float gyroBiasY = 0.0f;
static float gyroBiasZ = 0.0f;

// ── Sensor init ───────────────────────────────────────────────────────────

void sensors_init() {
  Wire.begin();

  // ── BMP280 ──
  if (!bmp.begin(BMP280_I2C_ADDR)) {
    Serial.println(F("ERROR: BMP280 not found. Check wiring and I2C address."));
    feedback_fatal_error();
  }

  // Recommended settings for dynamic (mobile) use:
  //   Oversampling x4 pressure, x1 temperature → ~8 ms conversion time (fits 40 ms loop)
  //   IIR filter coefficient 4 → smooths fast pressure spikes
  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X1,   // temperature oversampling
    Adafruit_BMP280::SAMPLING_X4,   // pressure oversampling
    Adafruit_BMP280::FILTER_X4,     // IIR filter
    Adafruit_BMP280::STANDBY_MS_1   // minimal standby → ~26 Hz output rate
  );

  // ── MPU6050 ──
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println(F("ERROR: MPU6050 not found. Check wiring and AD0 pin."));
    feedback_fatal_error();
  }

  mpu.setFullScaleAccelRange(ACCEL_FULL_SCALE);
  mpu.setFullScaleGyroRange(GYRO_FULL_SCALE);

  // Disable SLEEP mode (MPU6050 boots in sleep)
  mpu.setSleepEnabled(false);

  Serial.println(F("Sensors OK."));
}

// ── Ground calibration ────────────────────────────────────────────────────

void sensors_calibrate() {
  Serial.print(F("Calibrating ground pressure and gyro bias ("));
  Serial.print(CALIB_SAMPLES);
  Serial.print(F(" samples)..."));

  double pressureSum = 0.0;
  double gxSum = 0.0, gySum = 0.0, gzSum = 0.0;

  for (uint8_t i = 0; i < CALIB_SAMPLES; i++) {
    int16_t rawAx, rawAy, rawAz;
    int16_t rawGx, rawGy, rawGz;
    mpu.getMotion6(&rawAx, &rawAy, &rawAz, &rawGx, &rawGy, &rawGz);

    pressureSum += bmp.readPressure();
    gxSum += rawGx * GYRO_SCALE;
    gySum += rawGy * GYRO_SCALE;
    gzSum += rawGz * GYRO_SCALE;
    delay(40);
  }

  groundPressure_hPa = (float)(pressureSum / CALIB_SAMPLES) / 100.0f;
  gyroBiasX = (float)(gxSum / CALIB_SAMPLES);
  gyroBiasY = (float)(gySum / CALIB_SAMPLES);
  gyroBiasZ = (float)(gzSum / CALIB_SAMPLES);

  Serial.print(F(" ref = "));
  Serial.print(groundPressure_hPa, 2);
  Serial.println(F(" hPa"));
  Serial.print(F("Gyro bias - X: "));
  Serial.print(gyroBiasX, 3);
  Serial.print(F(" Y: "));
  Serial.print(gyroBiasY, 3);
  Serial.print(F(" Z: "));
  Serial.println(gyroBiasZ, 3);
}

// ── Per-sample read ───────────────────────────────────────────────────────

void sensors_read(SensorData *data) {
  data->timestamp_ms = millis();

  // BMP280: altitudeFromPressure uses the international barometric formula
  // with the stored ground-level reference so altitude reads 0 m at the pad.
  float pressurePa = bmp.readPressure();
  data->altitude_m  = 44330.0f * (1.0f - powf(
                        (pressurePa / 100.0f) / groundPressure_hPa, 0.1903f));

  // MPU6050: read all six axes in one I2C burst (6 registers, 12 bytes)
  int16_t rawAx, rawAy, rawAz;
  int16_t rawGx, rawGy, rawGz;
  mpu.getMotion6(&rawAx, &rawAy, &rawAz, &rawGx, &rawGy, &rawGz);

  data->ax = rawAx * ACCEL_SCALE;
  data->ay = rawAy * ACCEL_SCALE;
  data->az = rawAz * ACCEL_SCALE;
  data->gx = (rawGx * GYRO_SCALE) - gyroBiasX;
  data->gy = (rawGy * GYRO_SCALE) - gyroBiasY;
  data->gz = (rawGz * GYRO_SCALE) - gyroBiasZ;
}
