#include "voltage_sensor.h"

void VoltageSensor::begin() {
    pinMode(PIN_SOLAR_VOLTAGE, INPUT);

    analogReadResolution(12);

    // For ESP32-S3 Arduino core.
    // Gives suitable input headroom for ~0-3 V ADC measurements.
    analogSetPinAttenuation(PIN_SOLAR_VOLTAGE, ADC_11db);
}

void VoltageSensor::update() {
    const uint32_t now = millis();

    if (now - _lastSampleTime < VOLTAGE_SAMPLE_INTERVAL_MS) {
        return;
    }

    _lastSampleTime = now;

    const float newVoltage = readPanelVoltage();

    _samples[_sampleIndex] = newVoltage;
    _sampleIndex = (_sampleIndex + 1) % VOLTAGE_AVERAGE_SAMPLES;

    if (_sampleCount < VOLTAGE_AVERAGE_SAMPLES) {
        _sampleCount++;
    }

    float total = 0.0f;
    for (uint8_t i = 0; i < _sampleCount; i++) {
        total += _samples[i];
    }

    if (_sampleCount > 0) {
        _voltage = total / _sampleCount;
    }
}

float VoltageSensor::readPanelVoltage() {
    _rawADC = analogRead(PIN_SOLAR_VOLTAGE);

    // Calibrated ADC reading from the ESP32 Arduino core.
    const uint32_t millivolts = analogReadMilliVolts(PIN_SOLAR_VOLTAGE);
    _adcVoltage = millivolts / 1000.0f;

    const float dividerMultiplier =
        (VOLTAGE_R_TOP + VOLTAGE_R_BOTTOM) / VOLTAGE_R_BOTTOM;

    return _adcVoltage * dividerMultiplier * VOLTAGE_CALIBRATION;
}

float VoltageSensor::getVoltage() const {
    return _voltage;
}

float VoltageSensor::getADCVoltage() const {
    return _adcVoltage;
}

uint16_t VoltageSensor::getRawADC() const {
    return _rawADC;
}
