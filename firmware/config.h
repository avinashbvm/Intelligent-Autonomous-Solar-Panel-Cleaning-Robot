#pragma once
#include <Arduino.h>

// ============================================================
//                     WIFI SETTINGS
// ============================================================
#define WIFI_SSID       "YOUR_WIFI_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

// If STA Wi-Fi fails, ESP32 starts an access point.
#define FALLBACK_AP_SSID     "SolarCleaner"
#define FALLBACK_AP_PASSWORD "solarclean"

// ============================================================
//                       PIN SETTINGS
// ============================================================
// Change these to pins available on your ESP32-S3 board.
constexpr uint8_t PIN_SOLAR_VOLTAGE = 4;   // ADC-capable GPIO
constexpr uint8_t PIN_SERVO         = 6;
constexpr uint8_t PIN_PUMP          = 7;

// ============================================================
//                    VOLTAGE DIVIDER
// ============================================================
// Recommended for your current panel (max about 3 V):
//
// Solar + ---- 10k ----+---- ESP32 ADC pin
//                     |
//                    47k
//                     |
//                    GND
//
// Add a 100 nF capacitor from ADC pin to GND if possible.
//
// At 3.0 V:
// ADC ~= 3.0 * 47 / (10 + 47) = 2.47 V
//
// Divider multiplier = (10k + 47k) / 47k = 1.2127659
//
constexpr float VOLTAGE_R_TOP    = 10000.0f;
constexpr float VOLTAGE_R_BOTTOM = 47000.0f;

// Calibrate after assembly using a multimeter.
// New calibration = true_voltage / displayed_voltage
constexpr float VOLTAGE_CALIBRATION = 1.000f;

// ============================================================
//                    VOLTAGE SAMPLING
// ============================================================
constexpr uint32_t VOLTAGE_SAMPLE_INTERVAL_MS = 250;
constexpr uint8_t VOLTAGE_AVERAGE_SAMPLES = 12;
constexpr float MIN_VALID_PANEL_VOLTAGE = 0.20f;

// ============================================================
//                     SERVO SETTINGS
// ============================================================
constexpr uint8_t SERVO_HOME_ANGLE = 0;
constexpr uint8_t SERVO_END_ANGLE  = 180;
constexpr uint8_t SERVO_STEP_DEGREES = 2;
constexpr uint32_t SERVO_STEP_INTERVAL_MS = 15;

constexpr uint16_t SERVO_MIN_US = 500;
constexpr uint16_t SERVO_MAX_US = 2400;

// ============================================================
//                      PUMP SETTINGS
// ============================================================
constexpr uint32_t DEFAULT_DISPENSE_TIME_MS = 1200;
constexpr bool PUMP_ACTIVE_HIGH = true;
constexpr uint32_t MAX_PUMP_RUNTIME_MS = 10000;

// ============================================================
//                    CLEANING SETTINGS
// ============================================================
constexpr uint32_t WAIT_AFTER_DISPENSE_MS = 500;
constexpr uint8_t DEFAULT_WIPE_COUNT = 2;

// ============================================================
//                    AUTO CLEAN SETTINGS
// ============================================================

// Trigger auto clean after this percentage drop from baseline.
constexpr float DEFAULT_DROP_THRESHOLD_PERCENT = 12.0f;

// Voltage must stay low this long before cleaning.
constexpr uint32_t DROP_CONFIRMATION_TIME_MS = 15000;

// Minimum time before another automatic attempt.
constexpr uint32_t AUTO_CLEAN_COOLDOWN_MS = 30UL * 60UL * 1000UL;

// After cleaning, wait before evaluating whether output improved.
// This lets the panel and ADC settle.
constexpr uint32_t POST_CLEAN_SETTLE_MS = 8000;

// Required voltage improvement after cleaning.
// If improvement is smaller than this, assume the drop was caused
// by lighting/weather rather than dirt.
constexpr float MIN_CLEANING_IMPROVEMENT_PERCENT = 3.0f;

// Once classified as a light condition, stay locked out until
// voltage recovers sufficiently toward baseline.
constexpr float LIGHT_CONDITION_RECOVERY_MARGIN_PERCENT = 4.0f;

// ============================================================
//                       WEB SETTINGS
// ============================================================
constexpr uint16_t WEB_PORT = 80;
