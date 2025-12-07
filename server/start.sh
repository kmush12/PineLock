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

# Check and start Tailscale if installed
if command -v tailscale &> /dev/null; then
    echo "✅ Tailscale is installed"
    
    # Try to get Tailscale status (may require sudo)
    TAILSCALE_STATUS=$(tailscale status 2>&1 || true)
    
    if echo "$TAILSCALE_STATUS" | grep -q "Logged out"; then
        echo "⚠️  Tailscale is not authenticated. Please run: sudo tailscale up"
    elif echo "$TAILSCALE_STATUS" | grep -q "permission denied\|must be root"; then
        echo "⚠️  Tailscale requires sudo. Skipping Tailscale setup."
        echo "   To enable Tailscale, run: sudo tailscale up"
    else
        echo "✅ Tailscale is running"
        
        # Try to get hostname for public access
        if echo "$TAILSCALE_STATUS" | grep -q "100\."; then
            TAILSCALE_IP=$(echo "$TAILSCALE_STATUS" | grep -o "100\.[0-9.]*" | head -1)
            if [ -n "$TAILSCALE_IP" ]; then
                echo "🌐 Tailscale IP: $TAILSCALE_IP"
            fi
        fi
    fi
else
    echo "⚠️  Tailscale not installed. To install:"
    echo "   curl -fsSL https://tailscale.com/install.sh | sh"
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
