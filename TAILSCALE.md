# Tailscale Public Access Setup for PineLock

This guide explains how to set up public internet access to your PineLock UI using Tailscale Funnel.

## 🌐 What is Tailscale Funnel?

Tailscale Funnel allows you to expose your PineLock server to the public internet with a secure HTTPS URL, without port forwarding or complex networking setup.

**Your PineLock will be accessible at:** `https://your-hostname.tail<number>.ts.net`

## 📋 Prerequisites

- Raspberry Pi (or any Linux system) with PineLock installed
- Internet connection
- Email address for Tailscale account: `mrmaster622@gmail.com`

## 🚀 Quick Setup (One-Time)

### Step 1: Install and Configure Tailscale Funnel

Run the setup script:

```bash
cd /home/pi/PineLock/server
./tailscale_setup.sh
```

This script will:
1. Install Tailscale (if not already installed)
2. Authenticate with your Tailscale account (you'll need to open a link in your browser)
3. Configure Tailscale Funnel for public access on port 443 → localhost:8000
4. Display your public URL

### Step 2: Configure Auto-Start on Boot

To make PineLock and Tailscale Funnel start automatically when you boot your Raspberry Pi:

```bash
cd /home/pi/PineLock/server
./setup_autostart.sh
```

This script will:
1. Create systemd services for PineLock server and Tailscale Funnel
2. Enable them to start on boot
3. Start the services immediately

## ✅ Verification

After setup, your PineLock UI should be accessible at:

```
https://<your-hostname>.tail<number>.ts.net
```

**Login credentials:**
- Username: `admin`
- Password: `wkswks12`

## 🔧 Useful Commands

### Check Service Status

```bash
# Check PineLock server status
sudo systemctl status pinelock

# Check Tailscale Funnel status
sudo systemctl status tailscale-funnel
sudo tailscale funnel status
```

### View Logs

```bash
# View PineLock logs (real-time)
sudo journalctl -u pinelock -f

# View Tailscale Funnel logs
sudo journalctl -u tailscale-funnel -f
```

### Restart Services

```bash
# Restart PineLock server
sudo systemctl restart pinelock

# Restart Tailscale Funnel
sudo systemctl restart tailscale-funnel
```

### Stop/Disable Auto-Start

```bash
# Stop services
sudo systemctl stop pinelock
sudo systemctl stop tailscale-funnel

# Disable auto-start on boot
sudo systemctl disable pinelock
sudo systemctl disable tailscale-funnel
```

### Disable Public Access

To stop public access while keeping Tailscale running:

```bash
sudo tailscale funnel off
# or
sudo systemctl stop tailscale-funnel
```

## 🔐 Security Considerations

⚠️ **IMPORTANT:** Your PineLock UI is now publicly accessible from the internet!

**Security recommendations:**

1. ✅ **Strong password** - Already set to `wkswks12` (change if needed in `/home/pi/PineLock/server/.env`)
2. 🔒 **HTTPS** - Automatically enabled by Tailscale Funnel
3. 📊 **Monitor access logs** - Check `/ui/access-logs` regularly
4. 🚫 **Disable when not needed** - Use `sudo tailscale funnel off` to disable public access

## 🛠️ Troubleshooting

### Tailscale not connecting

```bash
# Check Tailscale status
sudo tailscale status

# Reconnect
sudo tailscale up
```

### PineLock server not starting

```bash
# Check logs
sudo journalctl -u pinelock -n 50

# Check if port 8000 is in use
sudo lsof -i :8000

# Restart manually
cd /home/pi/PineLock/server
./start.sh
```

### Funnel not working

```bash
# Check Funnel status
sudo tailscale funnel status

# Restart Funnel
sudo systemctl restart tailscale-funnel

# Or manually start Funnel
sudo tailscale funnel --bg --https=443 --set-path=/ http://localhost:8000
```

## 📱 Accessing from Mobile/Other Devices

Simply open your browser and navigate to:

```
https://<your-hostname>.tail<number>.ts.net
```

The URL will be displayed after running `./tailscale_setup.sh`

## 🔄 Updating PineLock

When you update PineLock code:

```bash
cd /home/pi/PineLock
git pull  # or copy new files

# Restart the service
sudo systemctl restart pinelock
```

The Tailscale Funnel will continue working without any changes.

## 📚 Additional Resources

- [Tailscale Funnel Documentation](https://tailscale.com/kb/1223/tailscale-funnel/)
- [Tailscale Admin Console](https://login.tailscale.com/admin/machines)

## 🎯 What Happens on Raspberry Pi Boot?

When you insert the SD card and boot your Raspberry Pi:

1. ✅ System boots up
2. ✅ Network connects
3. ✅ Tailscale daemon starts automatically
4. ✅ PineLock server starts automatically (systemd service)
5. ✅ Tailscale Funnel starts automatically (systemd service)
6. ✅ Your PineLock UI is publicly accessible!

**No manual intervention required!** 🎉
