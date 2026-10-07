#include "cleaner.h"
#include "config.h"

Cleaner::Cleaner(Pump &pump, Wiper &wiper)
    : _pump(pump), _wiper(wiper) {
}

void Cleaner::begin() {
    _state = IDLE;
}

void Cleaner::clean(uint8_t wipeCount, uint32_t dispenseTimeMs) {
    if (_state != IDLE || wipeCount == 0) {
        return;
    }

    _wipeCount = wipeCount;
    _pump.dispense(dispenseTimeMs);
    _state = DISPENSING;
}

void Cleaner::update() {
    switch (_state) {
        case IDLE:
            break;

        case DISPENSING:
            if (!_pump.isRunning()) {
                _waitStartTime = millis();
                _state = WAIT_AFTER_DISPENSE;
            }
            break;

        case WAIT_AFTER_DISPENSE:
            if (millis() - _waitStartTime >= WAIT_AFTER_DISPENSE_MS) {
                _wiper.wipe(_wipeCount);
                _state = WIPING;
            }
            break;

        case WIPING:
            if (!_wiper.isRunning()) {
                _state = IDLE;
            }
            break;
    }
}

bool Cleaner::isRunning() const {
    return _state != IDLE;
}
