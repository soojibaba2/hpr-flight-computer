#include "logger.h"
#include "config.h"
#include "feedback.h"
#include <SD.h>
#include <Arduino.h>

// ── Module-private state ──────────────────────────────────────────────────

static File logFile;

// ── Helper: find the next unused file number ─────────────────────────────
// Scans for FLT_0000.CSV, FLT_0001.CSV … and returns the first index
// that does not already exist.  Stops at 9999; wraps to 0 if full (unlikely).
static uint16_t nextFileIndex() {
  char name[18];   // "FLT_NNNN.CSV\0" = 13 chars (buffer sized with headroom)
  for (uint16_t i = 0; i <= 9999; i++) {
    snprintf(name, sizeof(name), "FLT_%04u.CSV", i);
    if (!SD.exists(name)) return i;
  }
  return 0;
}

// ── logger_init ───────────────────────────────────────────────────────────

void logger_init() {
  if (!SD.begin(PIN_SD_CS)) {
    Serial.println(F("ERROR: SD card mount failed. Check CS pin and card format (FAT32)."));
    feedback_fatal_error();
  }

  // Build a unique filename
  char filename[13];   // 8.3 format: "FLT_NNNN.CSV"
  snprintf(filename, sizeof(filename), "FLT_%04u.CSV", nextFileIndex());

  logFile = SD.open(filename, FILE_WRITE);
  if (!logFile) {
    Serial.print(F("ERROR: Cannot open "));
    Serial.println(filename);
    feedback_fatal_error();
  }

  // CSV header
  logFile.println(F("timestamp_ms,altitude_m,ax_mps2,ay_mps2,az_mps2,gx_dps,gy_dps,gz_dps"));
  logFile.flush();

  Serial.print(F("Logging to "));
  Serial.println(filename);

  feedback_file_opened();
}

// ── logger_write ──────────────────────────────────────────────────────────
// Uses print() chaining rather than sprintf() to keep SRAM usage low.
// flush() is called unconditionally after every row (every 40 ms) so that
// a mid-flight power loss or brownout loses at most one sample rather than
// up to a second of data. This trades additional SD wear/write latency for
// data-loss safety, which is the right tradeoff for a single-use flight log.

void logger_write(const SensorData *data) {
  if (!logFile) {
    Serial.println(F("ERROR: Log file handle lost."));
    feedback_fatal_error();
  }

  logFile.print(data->timestamp_ms);  logFile.print(',');
  logFile.print(data->altitude_m, 2); logFile.print(',');
  logFile.print(data->ax, 3);         logFile.print(',');
  logFile.print(data->ay, 3);         logFile.print(',');
  logFile.print(data->az, 3);         logFile.print(',');
  logFile.print(data->gx, 2);         logFile.print(',');
  logFile.print(data->gy, 2);         logFile.print(',');
  logFile.println(data->gz, 2);

  logFile.flush();
}
