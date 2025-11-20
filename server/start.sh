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

# Start server
echo "Starting PineLock Server..."
echo "API will be available at http://0.0.0.0:8000"
echo "API docs at http://0.0.0.0:8000/docs"
echo ""

uvicorn app.main:app --host 0.0.0.0 --port 8000 --reload
