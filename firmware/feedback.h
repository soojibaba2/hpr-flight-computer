#pragma once
#include <stdint.h>

// Initialises LED and buzzer pins.
void feedback_init();

// Two short beeps + LED solid ON: signals successful boot.
void feedback_boot_ok();

// Three long beeps + LED rapid-blink: signals a fatal error.
// Call before halt; does not return.
void feedback_fatal_error();

// Single short beep + brief LED flash: acknowledge a new log file opened.
void feedback_file_opened();
