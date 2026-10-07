#include "wiper.h"
#include "config.h"

void Wiper::begin() {
    _servo.setPeriodHertz(50);
    _servo.attach(PIN_SERVO, SERVO_MIN_US, SERVO_MAX_US);

    _currentAngle = SERVO_HOME_ANGLE;
    _servo.write(_currentAngle);

    _state = IDLE;
}

void Wiper::wipe(uint8_t count) {
    if (count == 0 || _state != IDLE) {
        return;
    }

    _requestedWipes = count;
    _completedWipes = 0;
    _state = MOVING_FORWARD;
}

void Wiper::update() {
    if (_state == IDLE) {
        return;
    }

    const uint32_t now = millis();

    if (now - _lastMoveTime < SERVO_STEP_INTERVAL_MS) {
        return;
    }

    _lastMoveTime = now;

    if (_state == MOVING_FORWARD) {
        _currentAngle += SERVO_STEP_DEGREES;

        if (_currentAngle >= SERVO_END_ANGLE) {
            _currentAngle = SERVO_END_ANGLE;
            _state = MOVING_BACKWARD;
        }

        _servo.write(_currentAngle);
    }
    else if (_state == MOVING_BACKWARD) {
        _currentAngle -= SERVO_STEP_DEGREES;

        if (_currentAngle <= SERVO_HOME_ANGLE) {
            _currentAngle = SERVO_HOME_ANGLE;
            _completedWipes++;

            if (_completedWipes >= _requestedWipes) {
                _state = IDLE;
            } else {
                _state = MOVING_FORWARD;
            }
        }

        _servo.write(_currentAngle);
    }
}

bool Wiper::isRunning() const {
    return _state != IDLE;
}

uint8_t Wiper::getCurrentAngle() const {
    return (uint8_t)_currentAngle;
}

uint8_t Wiper::getCompletedWipes() const {
    return _completedWipes;
}
