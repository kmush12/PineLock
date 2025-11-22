#!/bin/bash
echo "Installing Mosquitto MQTT Broker..."
sudo apt-get update
sudo apt-get install -y mosquitto mosquitto-clients

echo "Enabling and starting Mosquitto service..."
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

echo "Checking Mosquitto status..."
systemctl status mosquitto --no-pager

echo "MQTT Setup Complete!"
