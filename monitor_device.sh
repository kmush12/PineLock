#!/bin/bash
# Activate virtual environment with PlatformIO
source server/venv/bin/activate

# Detect USB port
if [ -e /dev/ttyACM0 ]; then
    USB_PORT=/dev/ttyACM0
elif [ -e /dev/ttyUSB0 ]; then
    USB_PORT=/dev/ttyUSB0
else
    echo "❌ No USB device found."
    exit 1
fi

# Check permissions
if [ ! -r "$USB_PORT" ] || [ ! -w "$USB_PORT" ]; then
    echo "⚠️  Insufficient permissions for $USB_PORT"
    echo "🔒 Requesting sudo access to fix permissions..."
    sudo chmod 666 "$USB_PORT"
    if [ $? -ne 0 ]; then
        echo "❌ Failed to change permissions. Please run: sudo chmod 666 $USB_PORT"
        exit 1
    fi
    echo "✅ Permissions fixed"
fi

echo "🔌 Monitoring device on $USB_PORT..."
echo "   (Press Ctrl+C to exit)"
echo ""

# Run monitor using the venv's pio
# ESP32-C3 native USB often needs DTR/RTS manipulation to show output
pio device monitor --port $USB_PORT --baud 115200 --dtr 0 --rts 0
