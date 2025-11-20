# Firmware Setup Guide

## Prerequisites

- Seeed XIAO ESP32-C3 board
- USB-C cable for programming
- PlatformIO installed (`pip install platformio`)
- Hardware components (see main README)

## Hardware Assembly

### 1. I2C Bus (Shared by PCF8574 and DS3231)

Connect to ESP32-C3:
- SDA → GPIO 6
- SCL → GPIO 7
- VCC → 3.3V
- GND → GND

### 2. PCF8574 I2C Expander

Connect 4x4 matrix keypad to PCF8574:
- P0-P3: Row pins (R1-R4)
- P4-P7: Column pins (C1-C4)

PCF8574 I2C address: 0x20 (default)

### 3. RC522 RFID Reader

Connect to ESP32-C3 SPI:
- SDA/SS → GPIO 3
- SCK → GPIO 8
- MOSI → GPIO 5
- MISO → GPIO 4
- RST → GPIO 2
- VCC → 3.3V
- GND → GND

### 4. DS3231 RTC Module

Already connected via I2C (shared bus with PCF8574)
- Insert CR2032 battery for backup power

### 5. MOSFET Lock Control

Connect MOSFET module:
- Signal/IN → GPIO 10
- VCC → 5V (from power supply)
- GND → Common ground
- Load: 12V electromagnetic lock

**Circuit:**
```
12V Supply → Lock → MOSFET Drain
MOSFET Source → GND
ESP32 GPIO 10 → MOSFET Gate (via module)
```

## Software Setup

### 1. Install PlatformIO

```bash
# Using pip
pip install platformio

# Or using VS Code extension
# Install "PlatformIO IDE" from VS Code marketplace
```

### 2. Configure WiFi and MQTT

Edit `firmware/lock_node/include/config.h`:

```cpp
// WiFi Configuration
#define WIFI_SSID "YourWiFiNetwork"
#define WIFI_PASSWORD "YourWiFiPassword"

// MQTT Configuration
#define MQTT_BROKER "192.168.1.100"  // Raspberry Pi IP
#define MQTT_PORT 1883
#define MQTT_USERNAME "pinelock"
#define MQTT_PASSWORD "your_mqtt_password"

// Device Configuration
#define DEVICE_ID "lock_001"  // Unique ID for this lock
```

### 3. Build Firmware

```bash
cd firmware/lock_node
pio run
```

### 4. Upload to ESP32-C3

Connect ESP32-C3 via USB and upload:

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

### 2. Test RFID

1. Present an RFID card to the reader
2. Card UID will be displayed on serial monitor
3. Add the UID to the server database
4. Present card again - lock should unlock

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

### RC522 Not Reading
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
