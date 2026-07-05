#pragma once
#include <stdint.h>

// ── Sensor data record ────────────────────────────────────────────────────
// One row per 25 Hz tick.  All physical units:
//   altitude_m   metres above launch pad (AGL)
//   ax/ay/az     m/s²  (gravity included; subtract 9.81 from az at rest)
//   gx/gy/gz     degrees/second
struct SensorData {
  uint32_t timestamp_ms;   // millis() at sample time
  float    altitude_m;     // AGL metres
  float    ax, ay, az;     // acceleration  m/s²
  float    gx, gy, gz;     // angular rate  °/s
};

// Initialises Wire bus, BMP280, and MPU6050.
// Calls feedback_fatal_error() and halts if either sensor is absent.
void sensors_init();

// Averages CALIB_SAMPLES pressure readings to set the ground-level reference.
// Must be called after sensors_init(), while the rocket is stationary.
void sensors_calibrate();

// Reads both sensors and fills *data.  Timestamp is set inside this function.
void sensors_read(SensorData *data);
