# Server Setup Guide

## Prerequisites

- Raspberry Pi (3 or 4 recommended) with Raspberry Pi OS
- Python 3.8 or higher
- Network connectivity

## Installation Steps

### 1. Update System

```bash
sudo apt-get update
sudo apt-get upgrade -y
```

### 2. Install Python Dependencies

```bash
sudo apt-get install -y python3-pip python3-venv
```

### 3. Create Virtual Environment

```bash
cd /home/pi/PineLock/server
python3 -m venv venv
source venv/bin/activate
```

### 4. Install Python Packages

```bash
pip install -r requirements.txt
```

### 5. Install and Configure MQTT Broker

```bash
# Install Mosquitto
sudo apt-get install -y mosquitto mosquitto-clients

# Enable and start service
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Check status
sudo systemctl status mosquitto
```

### 6. Configure MQTT Authentication (Optional but Recommended)

```bash
# Create password file
sudo mosquitto_passwd -c /etc/mosquitto/passwd pinelock

# Edit Mosquitto config
sudo nano /etc/mosquitto/mosquitto.conf
```

Add these lines:
```
allow_anonymous false
password_file /etc/mosquitto/passwd
```

Restart Mosquitto:
```bash
sudo systemctl restart mosquitto
```

### 7. Configure Environment Variables

```bash
cp .env.example .env
nano .env
```

Update these values:
```
MQTT_BROKER_HOST=localhost
MQTT_BROKER_PORT=1883
MQTT_USERNAME=pinelock
MQTT_PASSWORD=your_password_here
SECRET_KEY=generate_a_random_secret_key
```

### 8. Initialize Database

The database will be automatically created on first run.

### 9. Run the Server

For development:
```bash
source venv/bin/activate
uvicorn app.main:app --host 0.0.0.0 --port 8000 --reload
```

For production:
```bash
source venv/bin/activate
uvicorn app.main:app --host 0.0.0.0 --port 8000 --workers 4
```

### 10. Set Up as System Service (Production)

Create service file:
```bash
sudo nano /etc/systemd/system/pinelock.service
```

Content:
```ini
[Unit]
Description=PineLock Server
After=network.target mosquitto.service

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/PineLock/server
Environment="PATH=/home/pi/PineLock/server/venv/bin"
ExecStart=/home/pi/PineLock/server/venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000
Restart=always

[Install]
WantedBy=multi-user.target
```

Enable and start service:
```bash
sudo systemctl daemon-reload
sudo systemctl enable pinelock
sudo systemctl start pinelock
sudo systemctl status pinelock
```

## Verify Installation

### Check API

```bash
curl http://localhost:8000/
curl http://localhost:8000/health
```

### Check MQTT

```bash
# Subscribe to test topic
mosquitto_sub -h localhost -t "pinelock/#" -v

# In another terminal, publish test message
mosquitto_pub -h localhost -t "pinelock/test" -m "Hello"
```

## Accessing the API

The API will be available at:
- Local: `http://localhost:8000`
- Network: `http://<raspberry-pi-ip>:8000`
- API Documentation: `http://<raspberry-pi-ip>:8000/docs`

## Firewall Configuration

If using UFW:
```bash
sudo ufw allow 8000/tcp   # API
sudo ufw allow 1883/tcp   # MQTT
sudo ufw enable
```

## Troubleshooting

### Check Logs

```bash
# Server logs
sudo journalctl -u pinelock -f

# MQTT logs
sudo journalctl -u mosquitto -f
```

### Common Issues

1. **Port already in use**: Change port in `.env` or stop conflicting service
2. **MQTT connection failed**: Check mosquitto is running and credentials are correct
3. **Database errors**: Ensure write permissions in server directory
4. **Import errors**: Activate virtual environment and reinstall requirements
