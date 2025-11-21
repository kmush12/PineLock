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
