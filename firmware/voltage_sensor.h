#pragma once
#include <Arduino.h>
#include "config.h"

class VoltageSensor {
public:
    void begin();
    void update();

    float getVoltage() const;
    float getADCVoltage() const;
    uint16_t getRawADC() const;

private:
    float readPanelVoltage();

    float _samples[VOLTAGE_AVERAGE_SAMPLES] = {0};
    uint8_t _sampleIndex = 0;
    uint8_t _sampleCount = 0;

    float _voltage = 0.0f;
    float _adcVoltage = 0.0f;
    uint16_t _rawADC = 0;

    uint32_t _lastSampleTime = 0;
};
