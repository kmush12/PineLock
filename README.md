# PineLock - IoT Lock System

Distributed access management for vacation cabins with one central server and smart door locks. PineLock issues time-bound, offline-capable access using PIN keypads and RFID cards.

## System Architecture

### Components

1. **Server** (Raspberry Pi)
   - FastAPI REST API
   - MQTT broker/client for device communication
   - SQLite database for access management
   - Python-based

2. **Lock Nodes** (Seeed XIAO ESP32-C3)
   - WiFi connectivity
   - MQTT client
   - PCF8574 I2C keypad interface
   - RC522 RFID reader
   - DS3231 RTC for offline time-based access
   - MOSFET-controlled 12V lock
   - Offline PIN/RFID authentication

## Hardware Requirements

### Server
- Raspberry Pi 3/4 or similar
- Internet connectivity
- SD card with Raspberry Pi OS

### Lock Node
- Seeed XIAO ESP32-C3 microcontroller
- PCF8574 I2C I/O expander (for keypad)
- 4x4 Matrix keypad
- RC522 RFID reader module
- DS3231 RTC module
- MOSFET module (for 12V lock control)
- 12V electromagnetic lock
- 5V power supply
- Breadboard and jumper wires

## Pin Connections (ESP32-C3)

| Component | Pin | ESP32-C3 GPIO |
|-----------|-----|---------------|
| PCF8574 SDA | SDA | GPIO 6 |
| PCF8574 SCL | SCL | GPIO 7 |
| DS3231 SDA | SDA | GPIO 6 |
| DS3231 SCL | SCL | GPIO 7 |
| RC522 SS | SS | GPIO 3 |
| RC522 RST | RST | GPIO 2 |
| RC522 MOSI | MOSI | GPIO 5 |
| RC522 MISO | MISO | GPIO 4 |
| RC522 SCK | SCK | GPIO 8 |
| MOSFET Gate | IN | GPIO 10 |

## Installation

### Server Setup

1. **Install dependencies:**
   ```bash
   cd server
   pip install -r requirements.txt
   ```

2. **Configure environment:**
   ```bash
   cp .env.example .env
   # Edit .env with your settings
   nano .env
   ```

3. **Install MQTT Broker (Mosquitto):**
   ```bash
   sudo apt-get update
   sudo apt-get install mosquitto mosquitto-clients
   sudo systemctl enable mosquitto
   sudo systemctl start mosquitto
   ```

4. **Run the server:**
   ```bash
   cd server
   uvicorn app.main:app --host 0.0.0.0 --port 8000
   ```

### Firmware Setup

1. **Install PlatformIO:**
   ```bash
   pip install platformio
   ```

2. **Configure device settings:**
   ```bash
   cd firmware/lock_node
   # Edit include/config.h with your WiFi and MQTT settings
   nano include/config.h
   ```

3. **Build and upload:**
   ```bash
   cd firmware/lock_node
   pio run --target upload
   ```

4. **Monitor serial output:**
   ```bash
   pio device monitor
   ```

## API Usage

### Register a Lock

```bash
curl -X POST "http://localhost:8000/api/v1/locks" \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "lock_001",
    "name": "Front Door",
    "location": "Main Entrance"
  }'
```

### Add PIN Code

```bash
curl -X POST "http://localhost:8000/api/v1/access-codes" \
  -H "Content-Type: application/json" \
  -d '{
    "lock_id": 1,
    "code": "1234",
    "name": "Guest Code",
    "is_active": true
  }'
```

### Add RFID Card

```bash
curl -X POST "http://localhost:8000/api/v1/rfid-cards" \
  -H "Content-Type: application/json" \
  -d '{
    "lock_id": 1,
    "card_uid": "A1B2C3D4",
    "name": "Admin Card",
    "is_active": true
  }'
```

### Lock/Unlock Command

```bash
curl -X POST "http://localhost:8000/api/v1/locks/1/command" \
  -H "Content-Type: application/json" \
  -d '{
    "action": "unlock"
  }'
```

### View Access Logs

```bash
curl "http://localhost:8000/api/v1/locks/1/access-logs"
```

## MQTT Topics

The system uses the following MQTT topic structure:

- `pinelock/{device_id}/command` - Server to device commands
- `pinelock/{device_id}/sync` - Sync request from server
- `pinelock/{device_id}/status` - Device status updates
- `pinelock/{device_id}/access` - Access events from device
- `pinelock/{device_id}/heartbeat` - Device heartbeat

## Features

### Server
- ✅ RESTful API for lock management
- ✅ MQTT communication with devices
- ✅ SQLite database for persistence
- ✅ Access code management
- ✅ RFID card management
- ✅ Access logging
- ✅ Remote lock control
- ✅ Device status monitoring

### Firmware
- ✅ WiFi connectivity
- ✅ MQTT communication
- ✅ PIN keypad support (via PCF8574)
- ✅ RFID card reading (RC522)
- ✅ RTC for time-based access (DS3231)
- ✅ Offline authentication
- ✅ 12V lock control (MOSFET)
- ✅ Automatic re-locking
- ✅ Access event reporting

## Security Considerations

1. **Change default credentials** in production
2. **Use HTTPS** for API in production
3. **Enable MQTT authentication** with username/password
4. **Use TLS** for MQTT in production
5. **Regularly update** access codes and RFID cards
6. **Monitor access logs** for suspicious activity
7. **Keep firmware updated** on all devices

## Development

### Running Tests (Server)

```bash
cd server
pytest tests/
```

### Debug Mode

Enable debug logging in `.env`:
```
LOG_LEVEL=DEBUG
```

## Troubleshooting

### Device not connecting to MQTT
- Check WiFi credentials in `config.h`
- Verify MQTT broker is running
- Check firewall settings
- Verify broker IP address

### Keypad not working
- Check PCF8574 I2C address (default 0x20)
- Verify I2C connections (SDA, SCL)
- Test with I2C scanner

### RFID not reading cards
- Check SPI connections
- Verify RC522 is powered (3.3V)
- Test with example RFID sketch

### RTC time incorrect
- Replace CR2032 battery in DS3231
- Set time using compile time option
- Check I2C connections

## License

See LICENSE file for details.

## Contributing

Contributions are welcome! Please open an issue or pull request
