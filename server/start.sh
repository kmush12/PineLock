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

# Start server
echo "Starting PineLock Server..."
echo "API: http://${API_HOST}:${API_PORT}"
echo "Docs: http://${API_HOST}:${API_PORT}/docs"
echo "UI: http://${API_HOST}:${API_PORT}/ui/login"
echo ""

uvicorn app.main:app --host "${API_HOST}" --port "${API_PORT}" --reload
