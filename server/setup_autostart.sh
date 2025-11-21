#!/bin/bash

# Raspberry Pi Auto-Start Setup Script for PineLock
# This script configures systemd services for automatic startup on boot

set -e

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║     🌲  PineLock - Raspberry Pi Auto-Start Setup  🔐         ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Check if running on Raspberry Pi
if [ ! -f /proc/device-tree/model ] || ! grep -q "Raspberry Pi" /proc/device-tree/model 2>/dev/null; then
    echo "⚠️  Warning: This doesn't appear to be a Raspberry Pi"
    echo "   Continuing anyway..."
    echo ""
fi

# Get the current user (should be 'pi' on Raspberry Pi)
CURRENT_USER=$(whoami)
INSTALL_DIR=$(pwd)

echo "📋 Configuration:"
echo "   User: $CURRENT_USER"
echo "   Install directory: $INSTALL_DIR"
echo ""

# Update service files with correct paths
echo "🔧 Updating service files with correct paths..."
sed -i "s|User=pi|User=$CURRENT_USER|g" pinelock.service
sed -i "s|/home/pi/PineLock/server|$INSTALL_DIR|g" pinelock.service

echo "✅ Service files updated"
echo ""

# Install systemd services
echo "📦 Installing systemd services..."
sudo cp pinelock.service /etc/systemd/system/
sudo cp tailscale-funnel.service /etc/systemd/system/

# Reload systemd
sudo systemctl daemon-reload

# Enable services
echo "🔄 Enabling services for auto-start on boot..."
sudo systemctl enable pinelock.service
sudo systemctl enable tailscale-funnel.service

echo "✅ Services enabled successfully"
echo ""

# Start services now
echo "🚀 Starting services..."
sudo systemctl start pinelock.service
sleep 3
sudo systemctl start tailscale-funnel.service

echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║              🎉  Auto-Start Setup Complete!  🎉                ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "✅ PineLock will now automatically start on boot!"
echo ""
echo "📊 Service Status:"
echo ""
sudo systemctl status pinelock.service --no-pager -l | head -10
echo ""
echo "🔧 Useful commands:"
echo "   Check PineLock status:  sudo systemctl status pinelock"
echo "   Check Funnel status:    sudo systemctl status tailscale-funnel"
echo "   View PineLock logs:     sudo journalctl -u pinelock -f"
echo "   View Funnel logs:       sudo journalctl -u tailscale-funnel -f"
echo "   Restart PineLock:       sudo systemctl restart pinelock"
echo "   Stop auto-start:        sudo systemctl disable pinelock"
echo ""
echo "🌐 Access your PineLock UI:"
TAILSCALE_HOSTNAME=$(sudo tailscale status --json 2>/dev/null | grep -o '"HostName":"[^"]*"' | cut -d'"' -f4 || echo "your-hostname")
echo "   https://$TAILSCALE_HOSTNAME"
echo ""
