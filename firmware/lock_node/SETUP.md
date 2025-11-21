<div align="center">

# 🔧 Hardware Setup Guide
## PineLock ESP32-C3 Assembly & Configuration

**Target Board:** Seeed XIAO ESP32-C3  
**Skill Level:** Intermediate  
**Est. Time:** 45-60 minutes

[![Hardware](https://img.shields.io/badge/hardware-ESP32--C3-blue)]() [![Complexity](https://img.shields.io/badge/complexity-moderate-yellow)]() [![Tested](https://img.shields.io/badge/status-verified-success)]()

---

</div>

## 📦 Prerequisites

### Hardware Components

| Component | Specification | Quantity | Notes |
|-----------|---------------|----------|-------|
| 🔲 **ESP32-C3** | Seeed XIAO ESP32-C3 | 1 | Main controller |
| 🔌 **USB Cable** | USB-C data cable | 1 | For programming |
| 🎹 **Keypad** | 4x4 matrix keypad | 1 | PIN entry |
| 🔧 **I2C Expander** | PCF8574 module | 1 | Keypad interface |
| 📡 **RFID Reader** | RC522 module (SPI) | 1 | Key detection |
| ⏰ **RTC** | DS3231 module | 1 | Timekeeping |
| 🔋 **Battery** | CR2032 coin cell | 1 | RTC backup |
| ⚡ **MOSFET** | Logic level MOSFET module | 1 | Lock control |
| 🔒 **Lock** | 12V electromagnetic lock | 1 | Physical lock |
| 🔌 **Power Supply** | 12V DC adapter | 1 | System power |
| 🔗 **Wires** | Dupont jumper wires | Set | Connections |

### Software Requirements

- 🖥️ **PlatformIO** - Build system (`pip install platformio`)
- 💻 **VS Code** (optional) - PlatformIO IDE extension
- 🐍 **Python 3.7+** - For PlatformIO
- 📡 **MQTT Broker** - Mosquitto or similar

---

## 🔌 Hardware Assembly

### Circuit Overview

```
┌──────────────────────────────────────────────────────────┐
│                     ESP32-C3 XIAO                        │
│  ┌────────────────────────────────────────────────────┐  │
│  │  GPIO 6 (SDA) ──────────┬──────── PCF8574 (I2C)   │  │
│  │  GPIO 7 (SCL) ──────────┴──────── DS3231 (I2C)    │  │
│  │  GPIO 3 (SS)   ────────────────── RC522           │  │
│  │  GPIO 4 (MISO) ────────────────── RC522           │  │
│  │  GPIO 5 (MOSI) ────────────────── RC522           │  │
│  │  GPIO 8 (SCK)  ────────────────── RC522           │  │
│  │  GPIO 2 (RST)  ────────────────── RC522           │  │
│  │  GPIO 10       ────────────────── MOSFET Module   │  │
│  └────────────────────────────────────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

### 1️⃣ I2C Bus Configuration

**Shared by PCF8574 (Keypad) and DS3231 (RTC)**

**Shared by PCF8574 (Keypad) and DS3231 (RTC)**

| Pin | Connection | Color Suggestion |
|-----|------------|------------------|
| SDA | GPIO 6 | 🔵 Blue |
| SCL | GPIO 7 | 🟡 Yellow |
| VCC | 3.3V | 🔴 Red |
| GND | Ground | ⚫ Black |

> ⚠️ **Important**: Both PCF8574 and DS3231 share the same I2C bus. Connect them in parallel.

---

### 2️⃣ PCF8574 I2C Expander (Keypad Interface)

**4x4 Matrix Keypad Connection**

| PCF8574 Pin | Function | Keypad Pin |
|-------------|----------|------------|
| P0 | Row 1 | R1 |
| P1 | Row 2 | R2 |
| P2 | Row 3 | R3 |
| P3 | Row 4 | R4 |
| P4 | Column 1 | C1 |
| P5 | Column 2 | C2 |
| P6 | Column 3 | C3 |
| P7 | Column 4 | C4 |

**I2C Address:** `0x20` (default)

<details>
<summary>🔍 <b>Keypad Layout Reference</b></summary>

```
┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ A │
├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ B │
├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ C │
├───┼───┼───┼───┤
│ * │ 0 │ # │ D │
└───┴───┴───┴───┘

* = Clear PIN
# = Submit PIN
A-D = Reserved
```

</details>

---

### 3️⃣ RC522 RFID Reader (Key Presence Detection)

**SPI Connection to ESP32-C3**

| RC522 Pin | ESP32-C3 Pin | GPIO | Description |
|-----------|--------------|------|-------------|
| SDA (SS) | GPIO 3 | 3 | Chip Select |
| SCK | GPIO 8 | 8 | SPI Clock |
| MOSI | GPIO 5 | 5 | Master Out |
| MISO | GPIO 4 | 4 | Master In |
| IRQ | - | - | Not connected |
| GND | GND | - | Ground |
| RST | GPIO 2 | 2 | Reset |
| VCC | 3.3V | - | Power |

> 💡 **Tip**: Use short wires for SPI connections to avoid signal integrity issues.

---

### 4️⃣ DS3231 RTC Module

**Real-Time Clock with Battery Backup**

Already connected via I2C bus (shared with PCF8574)

**Additional Setup:**
1. Insert CR2032 battery into battery holder
2. Set initial time on first boot (auto-set to compile time)
3. RTC will maintain time during power loss

```cpp
// Time is automatically set on first boot:
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

---

### 5️⃣ MOSFET Lock Control

**12V Electromagnetic Lock Driver**

| Connection | Pin/Wire | Notes |
|------------|----------|-------|
| Signal IN | GPIO 10 | From ESP32-C3 |
| VCC | 5V | MOSFET module power |
| GND | Common GND | Shared ground |
| MOSFET OUT+ | Lock + | To lock positive |
| MOSFET OUT- | 12V GND | To lock negative |

**Power Circuit:**
```
12V PSU (+) ──→ Lock (+)
Lock (-)    ──→ MOSFET Drain
MOSFET Source ──→ 12V PSU (-)
ESP32 GPIO10  ──→ MOSFET Gate (via module)
```

> 🔒 **Safety**: Ensure proper current rating for your lock. Typical locks draw 200-500mA.

---

## 💻 Software Setup

### 1️⃣ Install PlatformIO

**Option A: Command Line**
```bash
pip install platformio
```

**Option B: VS Code Extension**
1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install

---

### 2️⃣ Configure WiFi and MQTT

Edit `include/config.h`:

Edit `include/config.h`:

```cpp
// 🌐 WiFi Configuration
#define WIFI_SSID "YourWiFiNetwork"       // Your 2.4GHz network name
#define WIFI_PASSWORD "YourWiFiPassword"  // Network password

// 📡 MQTT Broker Configuration
#define MQTT_BROKER "192.168.1.100"       // Broker IP (e.g., Raspberry Pi)
#define MQTT_PORT 1883                     // Standard MQTT port
#define MQTT_USERNAME "pinelock"           // MQTT username (or "" if none)
#define MQTT_PASSWORD "your_mqtt_password" // MQTT password (or "" if none)

// 🔑 Device Configuration
#define DEVICE_ID "lock_001"  // ⚠️ MUST BE UNIQUE FOR EACH LOCK!
```

> 🔐 **Security Reminder**: Never commit real credentials to Git. Use `config.h.example` as template.

---

### 3️⃣ Build & Upload Firmware

**Using PlatformIO CLI:**
```bash
cd lock_node
pio run                    # Build only
pio run --target upload    # Build and upload
pio device monitor         # View serial output
```

**Using VS Code:**
1. Open `lock_node` folder in VS Code
2. Click PlatformIO icon in sidebar
3. Select "Upload" under env:esp32-c3
4. Click "Monitor" to view output

---

## 🧪 Testing & Verification

### Initial Boot Sequence

Expected serial output:
```
=== PineLock Firmware ===
Device ID: lock_001
Watchdog configured
PCF8574 initialized
RTC initialized
RFID initialized
Hardware initialization complete
WARNING: No default PIN configured. Add PINs via MQTT.
WiFi connected!
IP address: 192.168.1.XXX
MQTT connected!
Subscribed to topics
```

### Component Testing

#### ✅ Test 1: I2C Devices
```bash
# Both PCF8574 and DS3231 should be detected
# Check serial output for "PCF8574 initialized" and "RTC initialized"
```

#### ✅ Test 2: Keypad
```bash
# Press any key on keypad
# Should see: "Key pressed: X" in serial monitor
```

#### ✅ Test 3: RFID Reader
```bash
# Place RFID card near reader
# Should see: "RFID key detected in box: XXXXXXXX"
```

#### ✅ Test 4: Lock Control
```bash
# Via MQTT:
mosquitto_pub -h localhost -t "pinelock/lock_001/command" -m '{"action":"unlock"}'
# Lock should activate for 5 seconds then re-lock
```

#### ✅ Test 5: MQTT Communication
```bash
# Subscribe to all device topics:
mosquitto_sub -h localhost -t "pinelock/lock_001/#" -v

# You should see:
# - Heartbeat every 60 seconds
# - Status updates on state changes
# - Access events when PIN entered
```

---

## 🐛 Troubleshooting

<details>
<summary><b>❌ "PCF8574 not found" Error</b></summary>

**Solutions:**
1. Check I2C wiring (SDA=GPIO6, SCL=GPIO7)
2. Verify 3.3V power to PCF8574
3. Check I2C address (default 0x20)
4. Use I2C scanner to detect device:
   ```cpp
   Wire.begin(6, 7);
   for(byte i = 1; i < 127; i++) {
       Wire.beginTransmission(i);
       if(Wire.endTransmission() == 0) {
           Serial.printf("Found I2C device at 0x%02X\n", i);
       }
   }
   ```

</details>

<details>
<summary><b>❌ "RTC not found" Error</b></summary>

**Solutions:**
1. Verify I2C connections (shared with PCF8574)
2. Check CR2032 battery installation
3. Verify DS3231 module power (3.3V)
4. Check for I2C address conflicts

</details>

<details>
<summary><b>❌ RFID Not Detecting Cards</b></summary>

**Solutions:**
1. Verify SPI connections:
   - MISO = GPIO 4
   - MOSI = GPIO 5
   - SCK = GPIO 8
   - SS = GPIO 3
   - RST = GPIO 2
2. Check 3.3V power supply
3. Use shorter wires (max 10cm for SPI)
4. Test with MFRC522 library examples
5. Try different RFID cards

</details>

<details>
<summary><b>❌ WiFi Connection Failed</b></summary>

**Solutions:**
1. Verify SSID and password in config.h
2. Ensure 2.4GHz network (ESP32-C3 doesn't support 5GHz)
3. Check WiFi signal strength
4. Disable WiFi security temporarily for testing
5. Check for MAC address filtering on router

</details>

<details>
<summary><b>❌ MQTT Connection Failed</b></summary>

**Solutions:**
1. Verify broker IP address
2. Test broker: `mosquitto_sub -h [broker_ip] -t test`
3. Check firewall settings
4. Verify username/password (or use empty strings)
5. Ensure broker allows remote connections

</details>

<details>
<summary><b>❌ Watchdog Reset Loop</b></summary>

**Solutions:**
1. Check for blocking code in loop()
2. Increase WDT_TIMEOUT from 30 to 60 seconds
3. Ensure WiFi/MQTT don't block indefinitely
4. Add debug prints to locate hang point

</details>

<details>
<summary><b>❌ Lock Not Activating</b></summary>

**Solutions:**
1. Verify GPIO 10 connection to MOSFET
2. Check 12V power supply
3. Test MOSFET manually with 3.3V signal
4. Verify lock polarity (some locks are polarized)
5. Check current rating (lock may draw more than supply provides)

</details>

---

## 📊 Pin Summary Table

| Function | GPIO | Component | Notes |
|----------|------|-----------|-------|
| I2C SDA | 6 | PCF8574, DS3231 | Shared bus |
| I2C SCL | 7 | PCF8574, DS3231 | Shared bus |
| SPI SS | 3 | RC522 | Chip select |
| SPI MISO | 4 | RC522 | Data in |
| SPI MOSI | 5 | RC522 | Data out |
| SPI SCK | 8 | RC522 | Clock |
| RFID RST | 2 | RC522 | Reset |
| Lock Control | 10 | MOSFET | PWM capable |

---

## 🔋 Power Requirements

| Component | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| ESP32-C3 | 3.3V | ~200mA | Via USB or regulator |
| PCF8574 | 3.3V | ~1mA | Low power |
| DS3231 | 3.3V | ~0.2mA | Battery backup |
| RC522 | 3.3V | ~13-26mA | During read |
| Lock | 12V | 200-500mA | Check your model |
| **Total** | **12V** | **~1A** | **Recommended: 2A supply** |

**Recommended Setup:**
- 12V 2A power supply
- 12V → Buck converter → 5V → ESP32-C3 (3.3V regulated onboard)
- 12V → Direct to lock via MOSFET

---

## 🎯 Next Steps

After successful hardware setup:

1. 📖 Read [DEPLOYMENT.md](DEPLOYMENT.md) for production deployment
2. 🧪 Run full test suite (see DEPLOYMENT.md)
3. 🔐 Add PIN codes and RFID cards
4. 📊 Configure server/backend for MQTT
5. 🏠 Install in final location

---

<div align="center">

### 🌲 PineLock Hardware Setup Complete!

**Ready for firmware deployment** ✅

[← Back to README](README.md) | [Deployment Guide →](DEPLOYMENT.md)

</div>

```bash
pio run --target upload
```

### 5. Monitor Serial Output

```bash
pio device monitor
```

Expected output:
```
=== PineLock Firmware ===
Device ID: lock_001
PCF8574 initialized
RTC initialized
RFID initialized
Hardware initialization complete
Connecting to WiFi: YourWiFiNetwork
...
WiFi connected!
IP address: 192.168.1.XXX
Attempting MQTT connection...connected!
Subscribed to topics
Setup complete!
```

## Testing

### 1. Test Default PIN

The firmware includes a default PIN code `1234` for testing:

1. Enter `1234#` on the keypad
2. Lock should unlock
3. Check serial monitor for confirmation
4. Lock should auto-lock after 5 seconds

### 2. Test RFID Key Detection

1. Register an RFID key in the server database first
2. Place the registered key near the RFID reader
3. Serial monitor should show "Valid key present in box"
4. Server should receive MQTT message with key presence status
5. Remove the key - should show "RFID key removed from box"

### 3. Test MQTT Commands

Using mosquitto_pub:

```bash
# Unlock
mosquitto_pub -h localhost -t "pinelock/lock_001/command" \
  -m '{"action":"unlock"}'

# Lock
mosquitto_pub -h localhost -t "pinelock/lock_001/command" \
  -m '{"action":"lock"}'
```

### 4. Monitor MQTT Messages

```bash
mosquitto_sub -h localhost -t "pinelock/#" -v
```

You should see:
- Heartbeat messages every 60 seconds
- Status updates when lock state changes
- Access events when PIN/RFID is used

## I2C Scanner (Troubleshooting)

If devices aren't detected, use this I2C scanner sketch:

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);  // SDA, SCL
  
  Serial.println("\nI2C Scanner");
  
  for(byte address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
}

void loop() {}
```

Expected devices:
- 0x20: PCF8574 (keypad)
- 0x68: DS3231 (RTC)

## Pin Configuration Quick Reference

```
ESP32-C3 Pin Mapping:
GPIO 2  → RC522 RST
GPIO 3  → RC522 SS
GPIO 4  → RC522 MISO
GPIO 5  → RC522 MOSI
GPIO 6  → I2C SDA (PCF8574, DS3231)
GPIO 7  → I2C SCL (PCF8574, DS3231)
GPIO 8  → RC522 SCK
GPIO 10 → MOSFET Gate (Lock Control)
```

## Troubleshooting

### WiFi Not Connecting
- Check SSID and password in config.h
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check WiFi signal strength

### MQTT Not Connecting
- Verify broker IP address
- Check MQTT credentials
- Ensure mosquitto is running on server
- Check firewall rules

### PCF8574 Not Found
- Verify I2C connections (SDA, SCL)
- Check I2C address with scanner
- Try different I2C address (0x20-0x27)

### RC522 Not Detecting Keys
- Check SPI connections
- Verify 3.3V power (not 5V!)
- Try different RFID cards
- Check RST and SS pin definitions

### RTC Issues
- Insert CR2032 battery
- Check I2C connections
- Time will be set to compile time on first boot

### Lock Not Operating
- Check MOSFET connections
- Verify 12V power supply
- Test MOSFET with multimeter
- Ensure common ground between ESP32 and 12V supply

## OTA Updates (Future Enhancement)

For remote firmware updates, add OTA support:

```cpp
#include <ArduinoOTA.h>

void setup() {
  // ... existing setup ...
  
  ArduinoOTA.setHostname(DEVICE_ID);
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  // ... existing loop ...
}
```

## Power Considerations

- ESP32-C3: ~80mA @ 3.3V (WiFi active)
- RC522: ~50mA @ 3.3V (active)
- PCF8574: ~1mA @ 3.3V
- DS3231: ~200μA @ 3.3V
- Lock: ~500mA @ 12V (when unlocked)

Recommended power supply:
- 5V 2A for ESP32 and modules
- 12V 1A for electromagnetic lock
- Separate power domains with common ground
