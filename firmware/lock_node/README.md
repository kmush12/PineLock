<div align="center">

# 🌲 PineLock Firmware

### *Secure. Smart. Simple.*

**ESP32-C3 Smart Lock System with Offline Authentication**

[![Platform](https://img.shields.io/badge/platform-ESP32--C3-blue.svg)](https://www.espressif.com/en/products/socs/esp32-c3)
[![Framework](https://img.shields.io/badge/framework-Arduino-00979D.svg)](https://www.arduino.cc/)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Status](https://img.shields.io/badge/status-Beta-yellow.svg)](CODE_REVIEW_REPORT.md)

---

</div>

## 📋 Overview

PineLock is a professional-grade smart lock firmware designed for ESP32-C3 based cabin locks ("domki") with robust offline authentication and cloud connectivity.

## ✨ Features

<table>
<tr>
<td width="50%">

### 🔐 **Security**
- 🔢 **PIN Authentication** - Secure 4x4 keypad entry
- 📇 **RFID Detection** - Key presence monitoring
- 💾 **Offline Storage** - EEPROM persistence
- ⏰ **Time-based Access** - Schedule-based control
- 🔒 **Auto-lock** - Configurable timeout

</td>
<td width="50%">

### 🌐 **Connectivity**
- 📡 **WiFi** - Local network integration
- 🔄 **MQTT** - Real-time server sync
- 📊 **Event Logging** - All access attempts tracked
- 💓 **Heartbeat** - Connection monitoring
- 🔔 **Status Updates** - Live lock state

</td>
</tr>
<tr>
<td width="50%">

### 🛠️ **Hardware**
- 🎯 **ESP32-C3** - RISC-V processor
- ⚡ **MOSFET Control** - 12V electromagnetic lock
- 🕐 **RTC DS3231** - Accurate timekeeping
- 📟 **PCF8574** - I2C keypad expander
- 📡 **RC522** - SPI RFID reader

</td>
<td width="50%">

### 🛡️ **Reliability**
- 🐕 **Watchdog Timer** - Auto-recovery
- 🔄 **Overflow Protection** - Long-term stability
- 💪 **Error Handling** - Robust validation
- 📝 **Comprehensive Logging** - Debug support
- ✅ **Production Ready** - Fully tested

</td>
</tr>
</table>

## 🚀 Quick Start

```bash
# 1️⃣ Configure your settings
# Edit include/config.h with your WiFi and MQTT credentials

# 2️⃣ Build and flash
cd lock_node
pio run --target upload

# 3️⃣ Monitor the system
pio device monitor
```

### ⚙️ Configuration Checklist

- [ ] Set unique `DEVICE_ID` in `config.h`
- [ ] Configure WiFi credentials (`WIFI_SSID`, `WIFI_PASSWORD`)
- [ ] Set MQTT broker address and credentials
- [ ] Verify GPIO pin assignments match your hardware
- [ ] Review timing parameters for your use case

> 📘 **Detailed Setup**: See [SETUP.md](SETUP.md) for complete hardware assembly and configuration instructions.

---

## ⚡ System Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| 🔒 Auto-lock delay | 5 seconds | Time before automatic re-lock |
| 💓 Heartbeat interval | 60 seconds | MQTT keep-alive frequency |
| 🎹 Keypad debounce | 500 ms | Key press stabilization |
| 📡 RFID check interval | 500 ms | Card detection frequency |
| 🐕 Watchdog timeout | 30 seconds | System hang recovery |
| 🔢 PIN length | 10 digits | Maximum length stored in EEPROM |

## File Structure

```
firmware/lock_node/
├── include/
│   ├── config.h              # Configuration settings
│   └── access_control.h      # Access control header
├── src/
│   ├── main.cpp              # Main firmware code
│   └── access_control.cpp    # Access control implementation
├── platformio.ini            # PlatformIO project configuration
├── SETUP.md                  # Detailed setup guide
└── README.md                 # This file
```

## 📡 MQTT Topics

### 📥 Subscribed Topics (Device Receives)

| Topic | Payload | Description |
|-------|---------|-------------|
| `pinelock/{device_id}/command` | `{"action": "lock\|unlock"}` | 🔐 Remote lock control |
| `pinelock/{device_id}/sync` | `{}` | 🔄 Trigger configuration sync |

#### Remote PIN Provisioning

Send to `pinelock/{device_id}/command`:

```json
{
  "action": "add_pin",
  "code": "567890",
  "active": true,
  "valid_from": 1732204800,
  "valid_until": 1732291200
}
```

- `code` – required, up to 10 digits (excess digits are rejected)
- `active` – optional, defaults to `true`
- `valid_from`/`valid_until` – optional Unix timestamps; include both to enforce a time window

Remove a code by publishing `{"action": "remove_pin", "code": "567890"}` to the same topic. Each successful change emits an `access` event with `access_type = admin_pin_*` so the backend can audit configuration edits.

### 📤 Published Topics (Device Sends)

| Topic | Payload Example | Frequency | Description |
|-------|----------------|-----------|-------------|
| `pinelock/{device_id}/status` | `{"is_locked": true, "is_key_present": false}` | On change | 🔔 Lock state updates |
| `pinelock/{device_id}/access` | `{"type": "pin", "success": true}` | On event | 📝 Access attempt logs |
| `pinelock/{device_id}/heartbeat` | `{"timestamp": 1732204800}` | Every 60s | 💓 Connection health check |

<details>
<summary>📋 <b>Full Topic Documentation</b></summary>

#### Status Message Schema
```json
{
  "is_locked": true,
  "is_key_present": false,
  "key_uid": "AB12CD34",  // Optional, if key present
  "timestamp": 1732204800
}
```

#### Access Event Schema
```json
{
  "access_type": "pin|remote",
  "access_method": "1234",  // PIN or "mqtt" for remote
  "success": true,
  "timestamp": 1732204800
}
```

#### Heartbeat Schema
```json
{
  "timestamp": 1732204800
}
```

</details>

## 🔧 Configuration

**Primary configuration file:** `include/config.h`

```cpp
// 🌐 Network Settings
#define WIFI_SSID "YourNetwork"
#define WIFI_PASSWORD "YourPassword"
#define MQTT_BROKER "192.168.1.100"

// 🔑 Device Identity
#define DEVICE_ID "lock_001"  // ⚠️ Must be unique!

// 🔌 Hardware GPIO Pins
#define I2C_SDA 6
#define I2C_SCL 7
#define RFID_SS_PIN 3
#define LOCK_MOSFET_PIN 10
// ... see config.h for complete list
```

> 🔒 **Security Note**: Never commit real credentials to version control. Use `config.h.example` as template.

---

## 👨‍💻 Development Guide

### Adding PIN Codes Programmatically

```cpp
// Simple PIN (always active)
accessControl.addPINCode("5678", true, false, DateTime(), DateTime());

// Time-limited PIN
DateTime validFrom(2025, 1, 1, 0, 0, 0);
DateTime validUntil(2025, 12, 31, 23, 59, 59);
accessControl.addPINCode("9999", true, true, validFrom, validUntil);
```

### Adding RFID Cards

```cpp
// Register a new RFID card
accessControl.addRFIDCard("A1B2C3D4", true, false, DateTime(), DateTime());

// Cards are automatically saved to EEPROM
```

### Persistence Operations

```cpp
// Manual save to EEPROM (auto-saved on add/remove)
accessControl.saveToEEPROM();

// Load from EEPROM (auto-loaded on startup)
accessControl.loadFromEEPROM();
```

---

## 🐛 Troubleshooting

<details>
<summary><b>❌ Common Issues & Solutions</b></summary>

### WiFi Connection Failed
- ✅ Verify SSID and password in `config.h`
- ✅ Check WiFi signal strength
- ✅ Ensure 2.4GHz network (ESP32-C3 doesn't support 5GHz)

### MQTT Not Connecting
- ✅ Verify broker IP and port
- ✅ Check firewall settings
- ✅ Test broker with `mosquitto_pub`/`mosquitto_sub`

### Keypad Not Responding
- ✅ Check I2C connections (SDA=GPIO6, SCL=GPIO7)
- ✅ Verify PCF8574 address (default 0x20)
- ✅ Test with I2C scanner

### RFID Not Detecting Cards
- ✅ Verify SPI connections
- ✅ Check 3.3V power supply
- ✅ Test with MFRC522 example sketch

### Watchdog Resets
- ✅ Check for blocking operations in code
- ✅ Increase `WDT_TIMEOUT` if needed
- ✅ Review serial logs for hang location

**📖 Complete guide**: [SETUP.md](SETUP.md) | [DEPLOYMENT.md](DEPLOYMENT.md)

</details>
