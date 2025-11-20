# PineLock Firmware

Firmware for ESP32-C3 based lock nodes with offline authentication capability.

## Features

- **WiFi Connectivity**: Connects to local WiFi network
- **MQTT Communication**: Two-way communication with central server
- **PIN Keypad**: 4x4 matrix keypad via PCF8574 I2C expander
- **RFID Reader**: RC522 RFID reader for card-based access
- **Real-Time Clock**: DS3231 RTC for time-based access control
- **Lock Control**: MOSFET-controlled 12V electromagnetic lock
- **Offline Authentication**: Stores access codes and RFID cards locally
- **Auto-locking**: Automatically locks after configurable duration
- **Event Reporting**: Reports all access attempts to server

## Hardware

See [SETUP.md](SETUP.md) for detailed hardware setup instructions.

## Quick Start

1. Configure WiFi and MQTT settings in `include/config.h`
2. Build and upload: `pio run --target upload`
3. Monitor output: `pio device monitor`

## Default Settings

- **Default PIN**: 1234 (for testing only, remove in production)
- **Auto-lock delay**: 5 seconds
- **Heartbeat interval**: 60 seconds

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

## MQTT Topics

### Subscribed (Device receives)
- `pinelock/{device_id}/command` - Lock/unlock commands
- `pinelock/{device_id}/sync` - Sync request for access codes/cards

### Published (Device sends)
- `pinelock/{device_id}/status` - Lock status updates
- `pinelock/{device_id}/access` - Access event logs
- `pinelock/{device_id}/heartbeat` - Periodic heartbeat

## Configuration

All configuration is in `include/config.h`:

- WiFi credentials
- MQTT broker settings
- Device ID (must be unique per lock)
- GPIO pin assignments
- Timing parameters
- I2C addresses

## Troubleshooting

See [SETUP.md](SETUP.md) for detailed troubleshooting steps.

## Development

### Adding Access Codes Programmatically

```cpp
accessControl->addPINCode("5678", true, false, DateTime(), DateTime());
```

### Adding RFID Cards Programmatically

```cpp
accessControl->addRFIDCard("A1B2C3D4", true, false, DateTime(), DateTime());
```

### Time-based Access

```cpp
DateTime validFrom(2024, 1, 1, 0, 0, 0);
DateTime validUntil(2024, 12, 31, 23, 59, 59);
accessControl->addPINCode("9999", true, true, validFrom, validUntil);
```
