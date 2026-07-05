#pragma once
#include "sensors.h"

// Mounts the SD card, creates a uniquely-numbered log file, and writes
// the CSV header row.  Calls feedback_fatal_error() and halts on failure.
void logger_init();

// Appends one CSV row for *data.  Calls feedback_fatal_error() and halts
// if the SD card becomes unwriteable mid-flight.
void logger_write(const SensorData *data);
