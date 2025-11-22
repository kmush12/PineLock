#!/bin/bash

# PineLock Server Startup Script

cd "$(dirname "$0")"

# Check if virtual environment exists
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python3 -m venv venv
fi

# Activate virtual environment
source venv/bin/activate

# Install/update dependencies
echo "Checking dependencies..."
pip install -q -r requirements.txt

# Check if .env exists
if [ ! -f ".env" ]; then
    echo "Creating .env from .env.example..."
    cp .env.example .env
    echo "⚠️  Please edit .env with your configuration!"
    exit 1
fi

# Setup MQTT Broker (Mosquitto)
echo "Checking MQTT Broker (Mosquitto)..."
if ! command -v mosquitto &> /dev/null; then
    echo "📡 Mosquitto not found. Installing..."
    if [ -f "../setup_mqtt.sh" ]; then
        bash ../setup_mqtt.sh
    else
        sudo apt-get update
        sudo apt-get install -y mosquitto mosquitto-clients
        sudo systemctl enable mosquitto
        sudo systemctl start mosquitto
    fi
    echo "✅ Mosquitto installed and started"
else
    # Check if mosquitto is running
    if ! systemctl is-active --quiet mosquitto; then
        echo "🔧 Starting Mosquitto..."
        sudo systemctl start mosquitto
    fi
    echo "✅ Mosquitto is running"
fi

# Resolve host/port from configuration
API_HOST=$(python3 - <<'PY'
from app.config import settings
print(settings.api_host)
PY
)
API_PORT=$(python3 - <<'PY'
from app.config import settings
print(settings.api_port)
PY
)

# Get local network IP addresses
LOCAL_IPS=$(hostname -I | tr ' ' '\n' | grep -E '^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$' | head -3)

# Check and start Tailscale if not running
if ! command -v tailscale &> /dev/null; then
    echo "⚠️  Tailscale not installed. Installing..."
    sudo snap install tailscale
    echo "✅ Tailscale installed. Please authenticate with: sudo tailscale up"
    exit 1
fi

if ! sudo tailscale status &> /dev/null; then
    echo "🔗 Starting Tailscale..."
    sudo tailscale up
    echo "✅ Tailscale started"
else
    echo "✅ Tailscale is already running"
fi

# Configure Tailscale Funnel if not running
if ! sudo tailscale funnel status &> /dev/null; then
    echo "🔧 Starting Tailscale Funnel..."
    sudo tailscale funnel --bg --https=443 --set-path=/ http://localhost:${API_PORT}
    echo "✅ Tailscale Funnel started"
else
    echo "✅ Tailscale Funnel is already running"
fi

# Get Tailscale hostname for public access
TAILSCALE_STATUS=$(sudo tailscale status --json 2>&1)
if echo "$TAILSCALE_STATUS" | grep -q '"HostName"'; then
    TAILSCALE_HOSTNAME=$(echo "$TAILSCALE_STATUS" | grep -o '"HostName":"[^"]*"' | cut -d'"' -f4)
    if [ -n "$TAILSCALE_HOSTNAME" ]; then
        echo "🌐 Tailscale public access:"
        echo "   https://$TAILSCALE_HOSTNAME"
        echo ""
    fi
else
    echo "⚠️  Tailscale not connected or status failed. Run 'sudo tailscale up' to authenticate."
fi

# Start server
echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║           🌲  PineLock Server - Starting...  🔐               ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "📡 Server is running on:"
echo ""
echo "   Local:    http://localhost:${API_PORT}/ui/login"
echo "   Local:    http://127.0.0.1:${API_PORT}/ui/login"
echo ""

if [ -n "$LOCAL_IPS" ]; then
    echo "🌐 Network access (from other devices):"
    echo ""
    while IFS= read -r ip; do
        if [ -n "$ip" ]; then
            echo "   Network:  http://${ip}:${API_PORT}/ui/login"
        fi
    done <<< "$LOCAL_IPS"
    echo ""
fi

echo "📚 API Documentation:"
echo "   Swagger:  http://localhost:${API_PORT}/docs"
echo "   ReDoc:    http://localhost:${API_PORT}/redoc"
echo ""
echo "ℹ️  Default credentials: admin / admin"
echo ""
echo "════════════════════════════════════════════════════════════════"
echo ""

uvicorn app.main:app --host "${API_HOST}" --port "${API_PORT}" --reload
