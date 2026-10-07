#include "pump.h"
#include "config.h"

void Pump::begin() {
    pinMode(PIN_PUMP, OUTPUT);
    setPump(false);
}

void Pump::dispense(uint32_t durationMs) {
    if (durationMs == 0) {
        return;
    }

    if (durationMs > MAX_PUMP_RUNTIME_MS) {
        durationMs = MAX_PUMP_RUNTIME_MS;
    }

    setPump(true);
    _running = true;
    _stopTime = millis() + durationMs;
}

void Pump::update() {
    if (!_running) {
        return;
    }

    if ((int32_t)(millis() - _stopTime) >= 0) {
        stop();
    }
}

void Pump::stop() {
    setPump(false);
    _running = false;
}

void Pump::setPump(bool state) {
    if (PUMP_ACTIVE_HIGH) {
        digitalWrite(PIN_PUMP, state ? HIGH : LOW);
    } else {
        digitalWrite(PIN_PUMP, state ? LOW : HIGH);
    }
}

bool Pump::isRunning() const {
    return _running;
}

uint32_t Pump::getRemainingMs() const {
    if (!_running) {
        return 0;
    }

    const int32_t remaining = (int32_t)(_stopTime - millis());
    return remaining > 0 ? (uint32_t)remaining : 0;
}
