#pragma once
#include <Arduino.h>
#include "pump.h"
#include "wiper.h"

class Cleaner {
public:
    Cleaner(Pump &pump, Wiper &wiper);

    void begin();

    // Complete sequence:
    // dispense -> short wait -> wipe N times
    void clean(uint8_t wipeCount, uint32_t dispenseTimeMs);

    void update();

    bool isRunning() const;

private:
    Pump &_pump;
    Wiper &_wiper;

    enum State {
        IDLE,
        DISPENSING,
        WAIT_AFTER_DISPENSE,
        WIPING
    };

    State _state = IDLE;

    uint8_t _wipeCount = 0;
    uint32_t _waitStartTime = 0;
};
