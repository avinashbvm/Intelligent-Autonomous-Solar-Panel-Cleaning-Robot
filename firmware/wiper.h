#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>

class Wiper {
public:
    void begin();

    // One wipe = HOME -> END -> HOME.
    void wipe(uint8_t count);

    void update();

    bool isRunning() const;
    uint8_t getCurrentAngle() const;
    uint8_t getCompletedWipes() const;

private:
    Servo _servo;

    enum State {
        IDLE,
        MOVING_FORWARD,
        MOVING_BACKWARD
    };

    State _state = IDLE;

    int _currentAngle = 0;
    uint8_t _requestedWipes = 0;
    uint8_t _completedWipes = 0;

    uint32_t _lastMoveTime = 0;
};
