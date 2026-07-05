#include "feedback.h"
#include "config.h"
#include <Arduino.h>

// ── Internal helpers ──────────────────────────────────────────────────────

static void beep(uint16_t durationMs) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(durationMs);
  digitalWrite(PIN_BUZZER, LOW);
}

static void led(bool on) {
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

// ── Public API ────────────────────────────────────────────────────────────

void feedback_init() {
  pinMode(PIN_LED,    OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_LED,    LOW);
  digitalWrite(PIN_BUZZER, LOW);
}

void feedback_boot_ok() {
  // Two short beeps, LED on
  beep(80);  delay(100);
  beep(80);
  led(true);
}

void feedback_fatal_error() {
  // Three long beeps, LED rapid blink — then hang
  led(false);
  for (uint8_t i = 0; i < 3; i++) {
    beep(400);
    delay(200);
  }
  // Rapid LED blink forever so the operator knows it is in fault state
  while (true) {
    led(true);  delay(100);
    led(false); delay(100);
  }
}

void feedback_file_opened() {
  beep(40);
  led(true); delay(60); led(false); delay(60); led(true);
}
