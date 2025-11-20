#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// MQTT Configuration
#define MQTT_BROKER "192.168.1.100"
#define MQTT_PORT 1883
#define MQTT_USERNAME ""
#define MQTT_PASSWORD ""
#define MQTT_TOPIC_PREFIX "pinelock"

// Device Configuration
#define DEVICE_ID "lock_001"

// Hardware Pin Configuration
// I2C for PCF8574 (Keypad) and DS3231 (RTC)
#define I2C_SDA 6
#define I2C_SCL 7

// SPI for RC522 RFID
#define RFID_SS_PIN 3
#define RFID_RST_PIN 2

// Lock Control
#define LOCK_MOSFET_PIN 10  // GPIO pin for MOSFET control

// Keypad Configuration (PCF8574)
#define PCF8574_ADDRESS 0x20
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

// Timing Configuration
#define HEARTBEAT_INTERVAL 60000  // Send heartbeat every 60 seconds
#define LOCK_DURATION 5000         // Keep lock open for 5 seconds
#define MQTT_RECONNECT_DELAY 5000  // Wait 5 seconds before MQTT reconnect

// Access Control
#define MAX_PIN_CODES 50
#define MAX_RFID_CARDS 50
#define PIN_LENGTH 4

#endif // CONFIG_H
