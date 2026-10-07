# ☀️ Intelligent Autonomous Solar Panel Cleaning Robot

An **ESP32-S3-based autonomous solar panel cleaning system** that monitors panel voltage, detects sustained drops in output, controls a liquid pump and servo-driven wiper mechanism, and evaluates whether cleaning actually improved the panel output.

The system also provides a **browser-based web dashboard** for live monitoring and manual control.

---

## 📌 Project Overview

Dust and dirt accumulation on solar panels can reduce their electrical output and increase the need for regular manual maintenance.

This project implements a modular embedded system that attempts to automate the cleaning process using:

- **ESP32-S3** as the main controller
- Solar-panel voltage sensing through an ADC and voltage divider
- A **180° positional servo** driving a rack-and-pinion / wiper mechanism
- A small liquid pump for dispensing cleaning fluid
- Automatic cleaning based on sustained voltage drop
- Post-cleaning voltage verification
- A browser-based monitoring and control dashboard

A key part of the design is that a low panel voltage is **not automatically assumed to be caused by dirt**. After a cleaning cycle, the system checks whether the panel voltage actually improves. If it does not, the system can infer that the low output may instead be caused by cloud cover, shade, sun angle, or another lighting condition.

---

# ✨ Key Features

- ESP32-S3-based embedded control
- Modular C++ firmware architecture
- Solar-panel voltage measurement using ESP32 ADC
- Voltage averaging for more stable measurements
- Clean-panel baseline calibration
- Automatic cleaning based on voltage-drop detection
- 15-second low-voltage confirmation
- 30-minute automatic-cleaning cooldown
- Servo-controlled wiper mechanism
- Pump-controlled cleaning-fluid dispensing
- Configurable number of wiping cycles
- Post-cleaning voltage comparison
- Automatic lighting-condition lockout
- Voltage recovery-based lockout release
- Browser-based live dashboard
- Manual wipe, dispense, and cleaning controls
- Wi-Fi station mode with fallback ESP32 access-point mode
- Non-blocking subsystem updates

---

# 🧠 System Architecture

```text
                  ┌───────────────────────┐
                  │     Solar Panel       │
                  └───────────┬───────────┘
                              │
                              ▼
                  ┌───────────────────────┐
                  │   Voltage Divider     │
                  │       10k / 47k       │
                  └───────────┬───────────┘
                              │
                              ▼
                  ┌───────────────────────┐
                  │      ESP32-S3 ADC     │
                  │   Voltage Monitoring  │
                  └───────────┬───────────┘
                              │
                              ▼
                  ┌───────────────────────┐
                  │   Auto-Clean Manager  │
                  │   Decision / State     │
                  │       Machine         │
                  └──────┬────────┬───────┘
                         │        │
              ┌──────────┘        └──────────┐
              ▼                              ▼
    ┌───────────────────┐          ┌───────────────────┐
    │   Pump Control    │          │   Wiper Control   │
    │ Cleaning Liquid   │          │  Servo + Wiper    │
    └─────────┬─────────┘          └─────────┬─────────┘
              │                              │
              └──────────────┬───────────────┘
                             ▼
                  ┌───────────────────────┐
                  │  Cleaning Mechanism   │
                  └───────────────────────┘

                             │
                             ▼
                  ┌───────────────────────┐
                  │     Web Dashboard     │
                  │ Monitoring + Control  │
                  └───────────────────────┘
```

---

# 🧩 Firmware Architecture

The firmware is divided into independent modules instead of placing the complete application inside a single source file.

```text
SolarCleaner_ESP32S3/
│
├── SolarCleaner_ESP32S3.ino
├── config.h
│
├── voltage_sensor.h
├── voltage_sensor.cpp
│
├── pump.h
├── pump.cpp
│
├── wiper.h
├── wiper.cpp
│
├── cleaner.h
├── cleaner.cpp
│
├── auto_clean.h
├── auto_clean.cpp
│
├── web_ui.h
├── web_ui.cpp
│
└── README.md
```

### Module responsibilities

| Module | Responsibility |
|---|---|
| `SolarCleaner_ESP32S3.ino` | Main application and subsystem initialization |
| `config.h` | Pins, thresholds, timing and system parameters |
| `voltage_sensor` | ADC sampling, averaging and panel-voltage calculation |
| `pump` | Cleaning-fluid pump control |
| `wiper` | Servo-based wiper movement |
| `cleaner` | Coordinates pump and wiper into a cleaning cycle |
| `auto_clean` | Automatic-cleaning decision logic |
| `web_ui` | Browser dashboard and HTTP API |

---

# ⚡ Solar Panel Voltage Sensing

The system measures the solar-panel voltage using an ESP32-S3 ADC input.

The current configuration uses:

```text
R1 = 10 kΩ
R2 = 47 kΩ
```

The voltage divider is:

```text
Solar Panel +
      │
     10kΩ
      │
      ├────────── ESP32-S3 ADC
      │
     47kΩ
      │
     GND
```

The divider multiplier used by the firmware is:

```text
(R1 + R2) / R2
```

With the current values:

```text
(10k + 47k) / 47k ≈ 1.2128
```

The firmware converts the ADC voltage back into the estimated panel voltage.

### Current ADC configuration

```cpp
PIN_SOLAR_VOLTAGE = 4
```

The firmware also supports voltage calibration through:

```cpp
VOLTAGE_CALIBRATION
```

A multimeter can be used to compare the measured panel voltage with the displayed value and adjust the calibration factor.

---

# 📊 Voltage Sampling

The voltage sensor module uses periodic sampling rather than relying on a single ADC reading.

Current configuration:

```cpp
VOLTAGE_SAMPLE_INTERVAL_MS = 250
VOLTAGE_AVERAGE_SAMPLES = 12
```

This provides a more stable voltage estimate for the automatic-cleaning decision.

The minimum valid panel voltage is currently:

```cpp
MIN_VALID_PANEL_VOLTAGE = 0.20 V
```

---

# 🧹 Automatic Cleaning Logic

The automatic-cleaning system is implemented in:

```text
auto_clean.cpp
auto_clean.h
```

The system first requires a **clean-panel baseline**.

The baseline can be established through the web dashboard using:

```text
Set Current as Baseline
```

The firmware then calculates:

```text
Voltage Drop (%) =
(Baseline Voltage - Current Voltage)
------------------------------------ × 100
       Baseline Voltage
```

---

## Automatic Cleaning Sequence

The decision process is:

```text
              Start Monitoring
                     │
                     ▼
             Measure Voltage
                     │
                     ▼
          Compare with Baseline
                     │
                     ▼
       Is voltage drop ≥ threshold?
              /              \
            No                Yes
            │                  │
            ▼                  ▼
      Keep monitoring   Start confirmation
                              timer
                                │
                                ▼
                    Has low voltage remained
                       for 15 seconds?
                         /          \
                       No            Yes
                       │              │
                       ▼              ▼
                   Monitor      Check cooldown
                                      │
                                      ▼
                              Start cleaning
```

---

# ⚙️ Current Automatic-Cleaning Parameters

The current firmware configuration contains:

| Parameter | Current value |
|---|---:|
| Voltage-drop trigger | **12%** |
| Low-voltage confirmation | **15 seconds** |
| Auto-clean cooldown | **30 minutes** |
| Post-clean settling time | **8 seconds** |
| Minimum cleaning improvement | **3%** |
| Light-condition recovery margin | **4%** |
| Default wipe count | **2 wipes** |
| Default pump duration | **1200 ms** |

These values are configurable in `config.h`.

---

# 🧴 Cleaning Cycle

The cleaning sequence is coordinated by:

```text
cleaner.cpp
```

When a cleaning operation begins:

```text
Cleaning requested
       │
       ▼
Dispense cleaning liquid
       │
       ▼
Wait 500 ms
       │
       ▼
Start wiper
       │
       ▼
Move servo toward 180°
       │
       ▼
Return servo toward 0°
       │
       ▼
Repeat configured wipe count
       │
       ▼
Cleaning complete
```

The current default configuration performs:

```text
2 wiping cycles
```

with:

```text
1200 ms
```

of pump operation.

---

# 🌀 Servo Wiper Control

The wiper mechanism uses a **180° positional servo**.

Current configuration:

```cpp
SERVO_HOME_ANGLE = 0°
SERVO_END_ANGLE  = 180°
```

The servo moves in:

```cpp
2° steps
```

with a:

```cpp
15 ms
```

step interval.

The wiper therefore performs a controlled forward-and-return movement rather than an instantaneous jump between positions.

The number of requested wiping cycles can be changed by the cleaning controller or through the web interface.

---

# 💧 Pump Control

The pump is controlled through a dedicated firmware module:

```text
pump.cpp
pump.h
```

The pump is activated for a configurable duration.

Current default:

```cpp
DEFAULT_DISPENSE_TIME_MS = 1200
```

A safety limit is also implemented:

```cpp
MAX_PUMP_RUNTIME_MS = 10000
```

The pump controller automatically stops the pump when the requested runtime expires.

---

# 🧠 Post-Cleaning Verification

One of the main features of the system is that cleaning is **evaluated after it finishes**.

Before cleaning:

```text
preCleanVoltage
```

is recorded.

After the cleaning cycle:

```text
8-second settling period
```

is allowed before measuring the post-cleaning voltage.

The firmware then calculates:

```text
Improvement (%) =
(Post-clean Voltage - Pre-clean Voltage)
----------------------------------------- × 100
           Pre-clean Voltage
```

---

## Example

If:

```text
Before cleaning = 2.55 V
After cleaning  = 2.72 V
```

then the voltage has increased after cleaning.

If the improvement is at least:

```text
3%
```

the system considers the cleaning result useful and returns to normal monitoring.

---

# ☁️ Lighting-Condition Detection / Lockout

A low panel voltage does not necessarily mean the panel is dirty.

For example:

```text
Cloud
Shade
Changing sun angle
Other lighting conditions
```

can also reduce the measured output.

Therefore, if the voltage does **not improve sufficiently after cleaning**, the firmware enters:

```text
LIGHT CONDITION LOCKOUT
```

During this state:

```text
Automatic cleaning is blocked.
```

This prevents repeated cleaning caused by a temporary reduction in sunlight.

---

# 🔄 Lockout Recovery

The system uses a recovery margin rather than immediately restarting automatic cleaning.

With the current configuration:

```text
Cleaning threshold = 12%
Recovery margin   = 4%
```

the recovery threshold becomes approximately:

```text
12% - 4% = 8%
```

Therefore, the system waits for the voltage drop to recover sufficiently toward the baseline before leaving the lighting-condition lockout.

This introduces hysteresis into the automatic-cleaning decision.

---

# 🌐 Web Dashboard

The ESP32-S3 hosts a browser-based dashboard.

The interface provides live information about the solar-panel cleaning system.

### Dashboard information

The current web interface reports:

- Panel voltage
- ADC voltage
- Raw ADC value
- Clean-panel baseline
- Voltage drop percentage
- Servo position
- Pump status
- Wiper status
- Cleaning status
- Automatic-cleaning status
- Lighting-condition lockout status
- Pre-clean voltage
- Post-clean voltage
- Cleaning improvement percentage
- Current automatic-cleaning state

---

# 🎛️ Manual Controls

The dashboard provides HTTP-based controls for:

```text
Wipe
Dispense
Clean Panel
Set Current as Baseline
Enable / Disable Auto Clean
```

The wipe endpoint also supports a configurable wipe count.

The dispense endpoint supports a configurable duration within the firmware's safety limit.

---

# 🔌 Web API

The firmware exposes the following HTTP endpoints:

| Endpoint | Method | Function |
|---|---|---|
| `/` | GET | Web dashboard |
| `/api/status` | GET | Live system status |
| `/api/wipe` | POST | Start wiper operation |
| `/api/dispense` | POST | Dispense cleaning liquid |
| `/api/clean` | POST | Start cleaning cycle |
| `/api/baseline` | POST | Set current voltage as baseline |
| `/api/auto` | POST | Enable/disable automatic cleaning |

The `/api/status` endpoint returns system telemetry in JSON format.

---

# 📡 Wi-Fi Operation

The ESP32-S3 initially attempts to connect to the configured Wi-Fi network.

If the connection succeeds:

```text
ESP32-S3
   │
   └── Wi-Fi Network
            │
            └── Browser Dashboard
```

If the configured Wi-Fi connection fails, the firmware starts a fallback access point:

```text
SSID: SolarCleaner
Password: solarclean
```

The fallback network allows the dashboard to remain accessible without an existing Wi-Fi network.

---

# 🔧 Pin Configuration

The current firmware configuration uses:

| Function | ESP32-S3 GPIO |
|---|---:|
| Solar voltage ADC | GPIO 4 |
| Servo | GPIO 6 |
| Pump | GPIO 7 |

These values are defined in:

```text
config.h
```

and can be changed according to the specific ESP32-S3 development board and hardware wiring.

---

# 🛠️ Software Requirements

The project is intended for:

- Arduino IDE 2.x
- ESP32 Arduino board package
- ESP32-S3 development board
- `ESP32Servo` library

### Board selection

A typical configuration is:

```text
Tools
→ Board
→ ESP32 Arduino
→ ESP32S3 Dev Module
```

The exact board selection should match the ESP32-S3 hardware being used.

---

# 🚀 Getting Started

## 1. Clone the repository

```bash
git clone https://github.com/avinashbvm/Intelligent-Autonomous-Solar-Panel-Cleaning-Robot.git
```

## 2. Open the firmware

Open:

```text
SolarCleaner_ESP32S3.ino
```

in Arduino IDE.

Make sure all `.cpp` and `.h` files remain in the same project directory.

## 3. Configure Wi-Fi

Open:

```text
config.h
```

and set:

```cpp
#define WIFI_SSID       "YOUR_WIFI_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"
```

**Do not commit real Wi-Fi credentials to GitHub.**

## 4. Check GPIO configuration

Verify:

```cpp
PIN_SOLAR_VOLTAGE
PIN_SERVO
PIN_PUMP
```

against the actual hardware.

## 5. Upload

Select the appropriate ESP32-S3 board and upload the firmware.

## 6. Open Serial Monitor

Use:

```text
115200 baud
```

The firmware reports Wi-Fi connection status and the dashboard IP address.

## 7. Establish a baseline

Before enabling automatic cleaning:

1. Clean the panel manually.
2. Place it under representative lighting.
3. Allow the voltage reading to stabilize.
4. Open the web dashboard.
5. Select **Set Current as Baseline**.

The recorded voltage becomes the reference for automatic-cleaning decisions.

---

# 📁 Repository Structure

```text
Intelligent-Autonomous-Solar-Panel-Cleaning-Robot/
│
├── README.md
│
├── firmware/
│   ├── SolarCleaner_ESP32S3.ino
│   ├── config.h
│   │
│   ├── voltage_sensor.h
│   ├── voltage_sensor.cpp
│   │
│   ├── pump.h
│   ├── pump.cpp
│   │
│   ├── wiper.h
│   ├── wiper.cpp
│   │
│   ├── cleaner.h
│   ├── cleaner.cpp
│   │
│   ├── auto_clean.h
│   ├── auto_clean.cpp
│   │
│   ├── web_ui.h
│   └── web_ui.cpp
│
├── hardware/
│   ├── circuit-diagram/
│   ├── block-diagram/
│   └── components/
│
├── images/
│   ├── prototype/
│   ├── dashboard/
│   └── testing/
│
└── documentation/
    ├── presentation/
    └── project-report/
```

---

# 🧪 Testing Approach

The current firmware supports testing the system in stages.

### Voltage sensing

Verify:

```text
Solar panel voltage
        ↓
Voltage divider
        ↓
ESP32 ADC
        ↓
Averaged measurement
        ↓
Dashboard
```

Compare the dashboard value with a multimeter and apply the calibration factor if required.

### Wiper testing

Use the dashboard's wipe control to verify:

- servo movement
- home position
- end position
- forward movement
- return movement
- multiple wipe cycles

### Pump testing

Use the dispense control and verify:

- pump activation
- pump runtime
- automatic stopping
- maximum runtime protection

### Cleaning sequence

Use the manual cleaning command to verify:

```text
Pump
 ↓
500 ms wait
 ↓
Wiper
 ↓
Cleaning complete
```

### Automatic cleaning

After establishing a baseline:

1. Enable automatic cleaning.
2. Monitor the voltage-drop percentage.
3. Verify the 15-second confirmation period.
4. Allow the automatic cleaning cycle to execute.
5. Observe the post-cleaning settling period.
6. Check the calculated voltage improvement.
7. Verify normal monitoring or lighting-condition lockout.

---

# ⚠️ Hardware Considerations

## Servo power

The servo should be powered from an appropriate external supply rather than directly from the ESP32-S3 3.3 V rail.

The ESP32-S3 ground and actuator supply ground should share a common reference where required by the circuit.

## Pump driver

The pump should **not be connected directly to an ESP32 GPIO**.

Use an appropriate transistor/MOSFET driver stage and suitable protection for the pump's electrical characteristics.

## Solar-panel ADC input

The voltage divider must keep the ESP32-S3 ADC input within its safe operating range.

The resistor values should be verified against the actual panel's maximum possible voltage before connecting the system.

---

# 🔒 Configuration & Safety

The following values are centralized in `config.h`:

```text
Wi-Fi settings
GPIO assignments
Voltage-divider values
ADC sampling
Voltage calibration
Servo limits
Servo movement speed
Pump runtime
Cleaning delay
Wipe count
Automatic-cleaning threshold
Confirmation time
Cooldown time
Post-cleaning settling time
Cleaning-improvement threshold
Lighting recovery margin
```

This makes the firmware easier to tune without modifying the core control modules.

---

# 📈 Current Design Characteristics

The current implementation demonstrates:

- Modular embedded firmware
- ADC-based sensor interfacing
- Sensor data processing
- Servo motor control
- Pump/actuator control
- State-machine-based sequencing
- Threshold-based decision making
- Hysteresis for automatic decisions
- Non-blocking subsystem updates
- Wi-Fi connectivity
- Embedded HTTP server
- JSON telemetry
- Browser-based hardware control

---

# 🚧 Current Limitations

The present implementation is a prototype-oriented system.

The actual cleaning effectiveness, mechanical reliability, and voltage-response behavior depend on:

- Solar-panel characteristics
- Lighting conditions
- Cleaning mechanism
- Wiper pressure and movement
- Pump flow
- Mechanical alignment
- Sensor calibration
- Environmental conditions

Voltage improvement is used as the current indicator for evaluating whether cleaning was useful. It should therefore be treated as a **prototype decision metric**, not as a direct measurement of panel efficiency.

---

# 🔮 Future Improvements

Potential future development includes:

- Improved mechanical structure
- Better wiper and rack-and-pinion optimization
- More robust environmental-condition detection
- Improved voltage and power measurement
- Current sensing
- Energy-consumption monitoring
- Battery and power-management integration
- Remote monitoring
- Data logging
- Cleaning-history storage
- More advanced cleaning-decision algorithms
- Long-duration outdoor testing
- Performance comparison before and after cleaning

---

# 🎯 Project Objective

The primary objective is to develop a practical and cost-effective embedded robotic system that can:

```text
Monitor
   ↓
Detect abnormal panel-output reduction
   ↓
Determine whether cleaning is appropriate
   ↓
Dispense cleaning liquid
   ↓
Operate the wiper mechanism
   ↓
Evaluate the result
   ↓
Resume monitoring or identify a lighting condition
```

The project combines **embedded systems, sensor interfacing, actuator control, automation, and web-based monitoring** into a single prototype.

---

# 👨‍💻 Author

**Avinash Raj**

B.Tech — Electronics and Communication Engineering  
VIT Bhopal University

### Technologies

`ESP32-S3` `Embedded C/C++` `Arduino` `ADC` `Sensors` `Servo Motor` `Pump Control` `Wi-Fi` `HTTP` `JSON` `Embedded Systems` `Robotics`

---

## 📜 License

This project is intended for educational, prototyping, and research purposes.

Add an appropriate open-source license to this repository if you intend to permit reuse or modification.sors` `Servo Motor` `Embedded Systems` `Robotics`
