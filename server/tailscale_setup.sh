#!/bin/bash

# Tailscale Installation and Setup Script for PineLock
# This script installs Tailscale and configures Funnel for public access

set -e

echo "╔════════════════════════════════════════════════════════════════╗"
echo "║        🌲  PineLock - Tailscale Funnel Setup  🔐             ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""

# Check if running as root
if [ "$EUID" -eq 0 ]; then 
   echo "⚠️  Please do not run this script as root"
   exit 1
fi

# Install Tailscale if not already installed
if ! command -v tailscale &> /dev/null; then
    echo "📦 Installing Tailscale..."
    sudo snap install tailscale
    echo "✅ Tailscale installed successfully"
else
    echo "✅ Tailscale is already installed"
fi

# Check if Tailscale is connected
if ! sudo tailscale status &> /dev/null; then
    echo ""
    echo "🔗 Tailscale is not connected. Please authenticate:"
    echo ""
    sudo tailscale up
    echo ""
    echo "✅ Tailscale connected successfully"
else
    echo "✅ Tailscale is already connected"
fi

# Get Tailscale hostname
TAILSCALE_HOSTNAME=$(sudo tailscale status --json | grep -o '"HostName":"[^"]*"' | cut -d'"' -f4)
TAILSCALE_IP=$(sudo tailscale ip -4)

echo ""
echo "📡 Tailscale Information:"
echo "   Hostname: $TAILSCALE_HOSTNAME"
echo "   IP: $TAILSCALE_IP"
echo ""

# Configure Tailscale Funnel for port 8000
echo "🔧 Configuring Tailscale Funnel for PineLock UI (port 8000)..."
echo ""

# Enable HTTPS
sudo tailscale cert "$TAILSCALE_HOSTNAME"

# Start Funnel on port 443 -> localhost:8000
sudo tailscale funnel --bg --https=443 --set-path=/ http://localhost:8000

echo ""
echo "✅ Tailscale Funnel configured successfully!"
echo ""
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                    🎉  Setup Complete!  🎉                     ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo ""
echo "🌐 Your PineLock UI is now publicly accessible at:"
echo ""
echo "   https://$TAILSCALE_HOSTNAME"
echo ""
echo "🔐 Login credentials:"
echo "   Username: admin"
echo "   Password: wkswks12"
echo ""
echo "⚠️  IMPORTANT: This URL is publicly accessible from the internet!"
echo "   Make sure to keep your credentials secure."
echo ""
echo "📊 To check Funnel status:"
echo "   sudo tailscale funnel status"
echo ""
echo "🛑 To stop Funnel:"
echo "   sudo tailscale funnel off"
echo ""
