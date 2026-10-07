#pragma once
#include <Arduino.h>
#include "voltage_sensor.h"
#include "cleaner.h"

class AutoCleanManager {
public:
    AutoCleanManager(VoltageSensor &sensor, Cleaner &cleaner);

    void begin();
    void update();

    void setEnabled(bool enabled);
    bool isEnabled() const;

    void setBaseline(float volts);
    void setBaselineFromCurrentVoltage();
    float getBaseline() const;

    void setDropThresholdPercent(float percent);
    float getDropThresholdPercent() const;

    float getDropPercent() const;

    // True when the system has inferred the low voltage is probably
    // caused by lighting/weather rather than dirt.
    bool isLightConditionLocked() const;

    float getPreCleanVoltage() const;
    float getPostCleanVoltage() const;
    float getLastImprovementPercent() const;

    const char* getStateText() const;

private:
    VoltageSensor &_sensor;
    Cleaner &_cleaner;

    enum State {
        MONITORING,
        WAITING_FOR_CLEAN_TO_FINISH,
        POST_CLEAN_SETTLING,
        LIGHT_CONDITION_LOCKOUT
    };

    State _state = MONITORING;

    bool _enabled = true;

    float _baselineVoltage = 0.0f;
    float _dropThresholdPercent = 0.0f;

    uint32_t _lowVoltageStartTime = 0;
    uint32_t _lastAutoCleanTime = 0;
    uint32_t _postCleanStartTime = 0;

    float _preCleanVoltage = 0.0f;
    float _postCleanVoltage = 0.0f;
    float _lastImprovementPercent = 0.0f;

    float calculateDropPercent(float currentVoltage) const;
    bool cooldownFinished() const;
};
