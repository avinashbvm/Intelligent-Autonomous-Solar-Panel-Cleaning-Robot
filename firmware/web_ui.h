#pragma once
#include <Arduino.h>
#include <WebServer.h>

#include "voltage_sensor.h"
#include "pump.h"
#include "wiper.h"
#include "cleaner.h"
#include "auto_clean.h"

class WebUI {
public:
    WebUI(
        VoltageSensor &sensor,
        Pump &pump,
        Wiper &wiper,
        Cleaner &cleaner,
        AutoCleanManager &autoClean
    );

    void begin();
    void update();

private:
    VoltageSensor &_sensor;
    Pump &_pump;
    Wiper &_wiper;
    Cleaner &_cleaner;
    AutoCleanManager &_autoClean;

    WebServer _server;

    void handleRoot();
    void handleStatus();
    void handleWipe();
    void handleDispense();
    void handleClean();
    void handleSetBaseline();
    void handleAutoToggle();
};
