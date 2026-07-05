#pragma once

// ── Timing ────────────────────────────────────────────────────────────────
#define SAMPLE_RATE_HZ    25
#define LOOP_PERIOD_MS    (1000 / SAMPLE_RATE_HZ)   // 40 ms

// ── Pin assignments ───────────────────────────────────────────────────────
#define PIN_SD_CS         10
#define PIN_LED            4   // active HIGH
#define PIN_BUZZER         5   // active HIGH

// ── Calibration ──────────────────────────────────────────────────────────
// Number of samples averaged to establish the ground-level pressure reference
#define CALIB_SAMPLES     32

// ── Serial stream ────────────────────────────────────────────────────────
// Set to 1 to echo every row to Serial (useful on the bench; wastes ~2 ms/row)
// SET BACK TO 0 BEFORE ANY ACTUAL FLIGHTS (since the Serial calls cost 2ms per row and you want the full 40ms budget clean)
#define SERIAL_STREAM      0

// ── BMP280 I2C address ───────────────────────────────────────────────────
// 0x76 when SDO pin is tied LOW (most breakouts); 0x77 when tied HIGH
#define BMP280_I2C_ADDR   0x76

// ── MPU6050 I2C address ──────────────────────────────────────────────────
// 0x68 when AD0 is tied LOW; 0x69 when AD0 is tied HIGH
#define MPU6050_I2C_ADDR  0x68

// ── Accel / gyro full-scale range ────────────────────────────────────────
// Accel: MPU6050_ACCEL_FS_2 / _4 / _8 / _16  (±g)
// Gyro : MPU6050_GYRO_FS_250 / _500 / _1000 / _2000  (°/s)
// For HPR: ±16 g accel, ±2000 °/s gyro
#define ACCEL_FULL_SCALE  MPU6050_ACCEL_FS_16
#define GYRO_FULL_SCALE   MPU6050_GYRO_FS_2000
