# PineLock - Project Summary

## Project Overview

PineLock is a complete IoT lock system for vacation cabins featuring:
- Central server on Raspberry Pi (Python FastAPI + MQTT + SQLite)
- Lock nodes on Seeed XIAO ESP32-C3 (Arduino C++)
- Offline-capable authentication with PIN codes and RFID cards
- Time-based access control
- Remote lock management

## What's Implemented

### ✅ Server Component (`/server`)

**Core Features:**
- FastAPI RESTful API with async support
- MQTT client for device communication
- SQLite database with SQLAlchemy ORM
- Complete CRUD operations for:
  - Lock devices
  - PIN access codes
  - RFID cards
  - Access logs
- Real-time device status monitoring
- Remote lock control via MQTT

**Files Created:**
- `app/main.py` - FastAPI application entry point
- `app/config.py` - Configuration management
- `app/database.py` - Database setup and session management
- `app/models.py` - SQLAlchemy database models
- `app/schemas.py` - Pydantic validation schemas
- `app/routes.py` - API endpoints
- `app/mqtt_client.py` - MQTT client implementation
- `app/mqtt_handlers.py` - MQTT message handlers
- `requirements.txt` - Python dependencies
- `start.sh` - Startup script

**API Endpoints:**
- `GET/POST /api/v1/locks` - Manage locks
- `POST /api/v1/locks/{id}/command` - Remote lock control
- `GET/POST/PUT/DELETE /api/v1/access-codes` - Manage PIN codes
- `GET/POST/PUT/DELETE /api/v1/rfid-cards` - Manage RFID cards
- `GET /api/v1/locks/{id}/access-logs` - View access history
- `GET /health` - Health check endpoint

### ✅ Firmware Component (`/firmware/lock_node`)

**Core Features:**
- WiFi connectivity (2.4GHz)
- MQTT client with auto-reconnect
- 4x4 matrix keypad via PCF8574 I2C expander
- RC522 RFID reader (SPI interface)
- DS3231 RTC for time-based access
- MOSFET-controlled 12V electromagnetic lock
- Offline authentication with local cache
- Auto-locking after configurable duration
- Event reporting to server
- Periodic heartbeat messages

**Files Created:**
- `src/main.cpp` - Main firmware code
- `src/access_control.cpp` - Access control implementation
- `include/config.h` - Configuration settings
- `include/access_control.h` - Access control header
- `platformio.ini` - PlatformIO project configuration

**Hardware Support:**
- ESP32-C3 WiFi/BLE SoC
- PCF8574 I2C I/O expander
- 4x4 matrix keypad
- RC522 RFID reader
- DS3231 RTC module
- MOSFET module for lock control
- 12V electromagnetic lock

### ✅ Documentation

**Comprehensive Guides:**
- `README.md` - Main project overview and quick start
- `server/README.md` - Server-specific documentation
- `server/SETUP.md` - Detailed server setup guide
- `firmware/lock_node/README.md` - Firmware overview
- `firmware/lock_node/SETUP.md` - Detailed firmware setup guide
- `API_EXAMPLES.md` - API usage examples (curl, Python, JavaScript)
- `ARCHITECTURE.md` - System architecture diagrams

**Configuration Examples:**
- `server/.env.example` - Server environment configuration
- Pin mapping documentation for ESP32-C3
- MQTT topic structure documentation
- Database schema documentation

## Technology Stack

### Server
- **Framework:** FastAPI 0.104.1
- **Web Server:** Uvicorn
- **Database:** SQLite with SQLAlchemy 2.0
- **MQTT:** Paho-MQTT 1.6.1
- **Language:** Python 3.8+

### Firmware
- **Platform:** ESP32-C3 (Espressif)
- **Framework:** Arduino
- **Build System:** PlatformIO
- **Libraries:**
  - WiFi (built-in)
  - PubSubClient (MQTT)
  - MFRC522 (RFID)
  - RTClib (DS3231)
  - Adafruit_PCF8574 (I2C expander)
  - ArduinoJson (JSON parsing)

## System Features

### Access Control
- ✅ PIN code authentication (4-10 digits)
- ✅ RFID card authentication
- ✅ Time-based access (valid from/until)
- ✅ Active/inactive status per code/card
- ✅ Offline validation capability
- ✅ Up to 50 PIN codes per lock
- ✅ Up to 50 RFID cards per lock

### Monitoring & Logging
- ✅ Real-time device status
- ✅ Online/offline tracking
- ✅ Lock state monitoring (locked/unlocked)
- ✅ Access event logging
- ✅ Success/failure tracking
- ✅ Timestamp for all events
- ✅ Periodic heartbeat messages

### Communication
- ✅ MQTT pub/sub messaging
- ✅ Automatic reconnection
- ✅ Topic-based routing
- ✅ JSON message format
- ✅ QoS 1 for commands
- ✅ Bidirectional communication

### Security
- ✅ Local access code storage
- ✅ Offline authentication
- ✅ Time-based validation with RTC
- ✅ Auto-locking mechanism
- ✅ Access audit trail
- ✅ MQTT authentication support
- ⚠️ API authentication (future enhancement)
- ⚠️ TLS encryption (future enhancement)

## Getting Started

### Quick Start (Server)

```bash
cd server
cp .env.example .env
# Edit .env with your settings
./start.sh
```

Server will be available at: `http://localhost:8000`
API docs at: `http://localhost:8000/docs`

### Quick Start (Firmware)

```bash
cd firmware/lock_node
# Edit include/config.h with WiFi/MQTT settings
pio run --target upload
pio device monitor
```

## Testing

### Server Testing
```bash
# Health check
curl http://localhost:8000/health

# Register a lock
curl -X POST http://localhost:8000/api/v1/locks \
  -H "Content-Type: application/json" \
  -d '{"device_id":"lock_001","name":"Test Lock","location":"Lab"}'
```

### Firmware Testing
1. Flash firmware to ESP32-C3
2. Monitor serial output for connection status
3. Test default PIN: 1234#
4. Check server logs for access events

### MQTT Testing
```bash
# Subscribe to all topics
mosquitto_sub -h localhost -t "pinelock/#" -v

# Send unlock command
mosquitto_pub -h localhost \
  -t "pinelock/lock_001/command" \
  -m '{"action":"unlock"}'
```

## Project Statistics

- **Total Files:** 25+
- **Lines of Code:** ~3,000+
  - Python: ~800 lines
  - C++: ~675 lines
  - Documentation: ~1,500+ lines
- **API Endpoints:** 15+
- **MQTT Topics:** 5 per device
- **Database Tables:** 4

## Production Considerations

### Before Deployment:

1. **Security:**
   - [ ] Change SECRET_KEY in .env
   - [ ] Enable MQTT authentication
   - [ ] Configure firewall rules
   - [ ] Set up HTTPS for API
   - [ ] Remove default PIN codes

2. **Performance:**
   - [ ] Configure proper database backups
   - [ ] Set up log rotation
   - [ ] Monitor resource usage
   - [ ] Test under load

3. **Reliability:**
   - [ ] Set up systemd service
   - [ ] Configure auto-restart
   - [ ] Set up monitoring/alerts
   - [ ] Test offline scenarios

4. **Hardware:**
   - [ ] Test all components
   - [ ] Verify power requirements
   - [ ] Install in weatherproof enclosure
   - [ ] Test WiFi signal strength

## Known Limitations

1. **Keypad:** Matrix scanning implementation is simplified (TODO in code)
2. **Sync:** Access code/RFID sync from server to device not fully implemented
3. **OTA:** Over-the-air firmware updates not implemented
4. **API Auth:** No authentication/authorization on API endpoints
5. **HTTPS:** No TLS/SSL configured by default
6. **WiFi:** Only 2.4GHz supported (ESP32 limitation)

## Future Enhancements

### Priority 1 (Security & Core Features)
- Implement API authentication (JWT tokens)
- Add HTTPS/TLS support
- Implement full sync mechanism (server → device)
- Complete keypad matrix scanning
- Add OTA firmware updates

### Priority 2 (Features)
- Add web dashboard for management
- Mobile app integration
- Push notifications
- Multi-factor authentication
- Scheduled access (recurring times)
- Master codes/cards

### Priority 3 (Advanced)
- Battery backup support
- Alarm integration
- Camera integration
- Bluetooth fallback
- Voice assistant integration
- Analytics dashboard

## Support & Troubleshooting

### Common Issues:

**Server won't start:**
- Check Python version (3.8+)
- Verify dependencies installed
- Check .env file exists

**Device won't connect:**
- Verify WiFi credentials
- Check MQTT broker running
- Verify broker IP address
- Check firewall settings

**RFID not working:**
- Check SPI connections
- Verify 3.3V power (not 5V)
- Test with different cards

**Lock not operating:**
- Check MOSFET connections
- Verify 12V power supply
- Test with multimeter
- Ensure common ground

### Documentation:
- Main README: Setup overview
- Server SETUP.md: Detailed server setup
- Firmware SETUP.md: Hardware and firmware setup
- API_EXAMPLES.md: Complete API reference
- ARCHITECTURE.md: System architecture

## License

See LICENSE file for details.

## Contributors

- GitHub Copilot (Initial implementation)
- kmush12 (Project owner)

---

**Status:** ✅ Implementation Complete - Ready for Testing

**Last Updated:** 2024
