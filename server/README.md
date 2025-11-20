# PineLock Server

Central management server for PineLock IoT lock system built with FastAPI, MQTT, and SQLite.

## Features

- **RESTful API**: Complete API for managing locks, access codes, and RFID cards
- **MQTT Broker Integration**: Real-time communication with lock devices
- **SQLite Database**: Lightweight database for access management
- **Access Logging**: Complete audit trail of all access attempts
- **Remote Control**: Lock/unlock devices remotely via API
- **Device Monitoring**: Track online status and last seen time
- **Time-based Access**: Support for time-limited access codes and cards
- **Web Dashboard**: Lightweight UI for monitoring nodes and managing PIN codes

## Quick Start

```bash
# Copy and configure environment
cp .env.example .env
nano .env

# Install dependencies
pip install -r requirements.txt

# Start server
./start.sh

# Or manually
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

## Web UI

- Panel logowania: http://localhost:8000/ui/login
- Domyślne dane logowania: `admin` / `admin`
- Dashboard pokazuje status lock nodes, ostatni kontakt oraz informację o kluczu
- Szczegóły węzła umożliwiają dodawanie i edycję kodów PIN

## API Documentation

Once running, visit:
- Interactive docs: http://localhost:8000/docs
- ReDoc: http://localhost:8000/redoc

## API Endpoints

### Locks
- `GET /api/v1/locks` - List all locks
- `POST /api/v1/locks` - Register new lock
- `GET /api/v1/locks/{id}` - Get lock details
- `PUT /api/v1/locks/{id}` - Update lock
- `DELETE /api/v1/locks/{id}` - Delete lock
- `POST /api/v1/locks/{id}/command` - Send lock/unlock command

### Access Codes
- `GET /api/v1/locks/{id}/access-codes` - List codes for lock
- `POST /api/v1/access-codes` - Create access code
- `PUT /api/v1/access-codes/{id}` - Update access code
- `DELETE /api/v1/access-codes/{id}` - Delete access code

### RFID Cards
- `GET /api/v1/locks/{id}/rfid-cards` - List cards for lock
- `POST /api/v1/rfid-cards` - Create RFID card
- `PUT /api/v1/rfid-cards/{id}` - Update RFID card
- `DELETE /api/v1/rfid-cards/{id}` - Delete RFID card

### Access Logs
- `GET /api/v1/locks/{id}/access-logs` - View access history

## File Structure

```
server/
├── app/
│   ├── __init__.py           # Package init
│   ├── main.py               # FastAPI application
│   ├── config.py             # Configuration management
│   ├── database.py           # Database setup
│   ├── models.py             # SQLAlchemy models
│   ├── schemas.py            # Pydantic schemas
│   ├── routes.py             # API routes
│   ├── mqtt_client.py        # MQTT client
│   ├── mqtt_handlers.py      # MQTT message handlers
│   ├── ui_routes.py          # Dashboard routes
│   ├── templates/            # Jinja2 templates
│   └── static/               # CSS assets
├── tests/                    # Test files
├── .env.example              # Example environment config
├── requirements.txt          # Python dependencies
├── start.sh                  # Startup script
├── SETUP.md                  # Detailed setup guide
└── README.md                 # This file
```

## Environment Variables

Key settings in `.env`:

```
MQTT_BROKER_HOST=localhost
MQTT_BROKER_PORT=1883
DATABASE_URL=sqlite+aiosqlite:///./locks.db
API_HOST=0.0.0.0
API_PORT=8000
ADMIN_USERNAME=admin
ADMIN_PASSWORD=admin
SESSION_SECRET_KEY=change-me
```

## MQTT Topics

### Server subscribes to:
- `pinelock/+/status` - Device status updates
- `pinelock/+/access` - Access events
- `pinelock/+/heartbeat` - Device heartbeats

### Server publishes to:
- `pinelock/{device_id}/command` - Lock commands
- `pinelock/{device_id}/sync` - Sync requests

## Database Schema

### Tables
- **locks**: Device registry
- **access_codes**: PIN codes
- **rfid_cards**: RFID card registry
- **access_logs**: Access attempt history

## Development

### Running with Auto-reload

```bash
uvicorn app.main:app --reload
```

### Database Migrations

Database is auto-created on startup. For migrations, consider using Alembic.

### Testing

```bash
pytest tests/
```

## Production Deployment

See [SETUP.md](SETUP.md) for:
- systemd service configuration
- MQTT authentication setup
- HTTPS configuration
- Security hardening

## Security Notes

⚠️ **Important for Production:**

1. Enable MQTT authentication
2. Use HTTPS for API
3. Configure firewall rules
4. Regular database backups
5. Monitor access logs
6. Keep dependencies updated

## Troubleshooting

### Server won't start
- Check Python version (3.8+)
- Verify all dependencies installed
- Check `.env` file exists

### MQTT connection fails
- Ensure mosquitto is running
- Check broker host/port
- Verify credentials if auth enabled

### Database errors
- Check file permissions
- Ensure directory is writable
- Delete database file to reset

## Support

See main project README for additional information and support.
