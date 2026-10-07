#include <Arduino.h>
#include <WiFi.h>

#include "config.h"
#include "voltage_sensor.h"
#include "pump.h"
#include "wiper.h"
#include "cleaner.h"
#include "auto_clean.h"
#include "web_ui.h"

VoltageSensor voltageSensor;
Pump pump;
Wiper wiper;
Cleaner cleaner(pump, wiper);
AutoCleanManager autoClean(voltageSensor, cleaner);
WebUI webUI(voltageSensor, pump, wiper, cleaner, autoClean);

void connectWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");

    const uint32_t start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 15000) {
        delay(300);
        Serial.print(".");
    }

    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Wi-Fi connected.");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
        return;
    }

    Serial.println("Wi-Fi failed. Starting fallback AP.");

    WiFi.mode(WIFI_AP);
    WiFi.softAP(FALLBACK_AP_SSID, FALLBACK_AP_PASSWORD);

    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("===================================");
    Serial.println(" ESP32-S3 Solar Cleaning Robot");
    Serial.println("===================================");

    voltageSensor.begin();
    pump.begin();
    wiper.begin();
    cleaner.begin();
    autoClean.begin();

    connectWiFi();
    webUI.begin();

    Serial.println("Web UI started.");
    Serial.println();
    Serial.println("IMPORTANT:");
    Serial.println("1. Clean the panel manually.");
    Serial.println("2. Put it under representative lighting.");
    Serial.println("3. Open the Web UI.");
    Serial.println("4. Press 'Set Current as Baseline'.");
}

void loop() {
    // All subsystems are non-blocking.
    voltageSensor.update();
    pump.update();
    wiper.update();
    cleaner.update();
    autoClean.update();
    webUI.update();

    delay(1);
}
