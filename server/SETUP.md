# Server Setup Guide

## Prerequisites

- **Raspberry Pi 5** with **Raspberry Pi OS (64-bit) Bookworm** or later
- **Python 3.9 or higher** (included with Raspberry Pi OS Bookworm)
- **Network connectivity** (Ethernet or WiFi)
- **MicroSD card** (32GB or larger recommended)
- **Power supply** (5V USB-C, 5A recommended for Pi 5)

## Operating System Installation

### Raspberry Pi OS for Raspberry Pi 5

1. **Download Raspberry Pi Imager** from [raspberrypi.com/software](https://www.raspberrypi.com/software/)

2. **Choose OS**: Select `Raspberry Pi OS (64-bit)` → `Raspberry Pi OS (64-bit)`

3. **Choose Storage**: Insert microSD card and select it

4. **Configure OS** (optional but recommended):
   - Set hostname: `pinelock-server`
   - Enable SSH
   - Set username and password
   - Configure WiFi (if not using Ethernet)

5. **Write to microSD card**

6. **Insert microSD into Raspberry Pi 5** and power on

### Monitor Connection (microHDMI)

Raspberry Pi 5 has **2x microHDMI ports** for video output:

- **HDMI0 (near USB ports)**: Primary display
- **HDMI1 (near Ethernet)**: Secondary display

**To connect a monitor:**
1. Use a **microHDMI to HDMI cable** (or adapter)
2. Connect to **HDMI0 port** (recommended)
3. Monitor should automatically detect the signal

**Note**: Raspberry Pi OS Bookworm includes proper microHDMI drivers for Pi 5.

## Installation Steps

### 1. Update System

```bash
sudo apt-get update
sudo apt-get upgrade -y
```

### 2. Verify Python Installation

Raspberry Pi OS Bookworm includes Python 3.11:

```bash
python3 --version  # Should show Python 3.11.x
pip3 --version     # Should show pip for Python 3.11
```

### 3. Install Python Dependencies

```bash
sudo apt-get install -y python3-pip python3-venv
```

### 4. Create Virtual Environment

```bash
cd /home/pi/PineLock/server
python3 -m venv venv
source venv/bin/activate
```

### 5. Install Python Packages

```bash
pip install -r requirements.txt
```

### 6. Install and Configure MQTT Broker

```bash
# Install Mosquitto
sudo apt-get install -y mosquitto mosquitto-clients

# Enable and start service
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Check status
sudo systemctl status mosquitto
```

### 7. Configure MQTT Authentication (Optional but Recommended)

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

### 8. Configure Environment Variables

```bash
cp .env.example .env
nano .env
```

Update these values:
```
MQTT_BROKER_HOST=localhost
MQTT_BROKER_PORT=1883
MQTT_USERNAME=pinelock
MQTT_PASSWORD=your_mqtt_password
ADMIN_USERNAME=admin
ADMIN_PASSWORD=strong_password
SESSION_SECRET_KEY=super_secret_key
```

### 9. Initialize Database

The database will be automatically created on first run.

### 10. Run the Server

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

### 11. Set Up as System Service (Production)

Create service file:
```bash
sudo nano /etc/systemd/system/pinelock.service
```

Content (adjust paths if using different username):
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
RestartSec=5

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

### Web UI

Open `http://<adres_serwera>:8000/ui/login` w przeglądarce i zaloguj się danymi z `.env`.

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
