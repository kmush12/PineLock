#!/bin/bash
echo "Uploading firmware to device..."

# Activate virtual environment with PlatformIO
source server/venv/bin/activate

# Navigate to firmware directory
cd firmware/lock_node

# Detect USB port
echo "Detecting USB port..."
if [ -e /dev/ttyACM0 ]; then
    USB_PORT=/dev/ttyACM0
elif [ -e /dev/ttyUSB0 ]; then
    USB_PORT=/dev/ttyUSB0
else
    echo "❌ No USB device found. Please connect the device and try again."
    echo "   Looking for /dev/ttyACM0 or /dev/ttyUSB0"
    exit 1
fi

echo "✅ Found device on $USB_PORT"

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

# Upload firmware
echo "Uploading firmware to domek_1..."
pio run -t upload --upload-port $USB_PORT

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Upload complete!"
    echo "You can monitor the device with: ./monitor_device.sh"
else
    echo ""
    echo "❌ Upload failed!"
    echo "Try putting the device in bootloader mode:"
    echo "1. Hold the BOOT button (left button)"
    echo "2. Press and release RESET (right button)"
    echo "3. Release BOOT button"
    echo "4. Try uploading again"
    exit 1
fi

