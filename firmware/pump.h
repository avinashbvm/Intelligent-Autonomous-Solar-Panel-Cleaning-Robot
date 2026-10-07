#pragma once
#include <Arduino.h>

class Pump {
public:
    void begin();

    // Non-blocking.
    void dispense(uint32_t durationMs);

    void update();
    void stop();

    bool isRunning() const;
    uint32_t getRemainingMs() const;

private:
    void setPump(bool state);

    bool _running = false;
    uint32_t _stopTime = 0;
};
