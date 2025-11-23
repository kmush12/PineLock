#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration - SET THESE BEFORE DEPLOYMENT!
#define WIFI_SSID "Orange_Swiatlowod_DDC0"          // Your WiFi network name
#define WIFI_PASSWORD "M3WGS27SRMY6"      // Your WiFi password

// MQTT Configuration - SET THESE BEFORE DEPLOYMENT!
#define MQTT_BROKER "192.168.1.15"        // IP address of MQTT broker (e.g., "192.168.1.100")
#define MQTT_PORT 1883
#define MQTT_USERNAME ""      // MQTT username (leave empty if no auth)
#define MQTT_PASSWORD ""      // MQTT password (leave empty if no auth)
#define MQTT_TOPIC_PREFIX "pinelock"

// Device Configuration - SET UNIQUE ID FOR EACH LOCK!
#define DEVICE_ID "domek_1"  // Unique identifier for this lock device

// Hardware Pin Configuration
// I2C for PCF8574 (Keypad) and DS3231 (RTC)
#define I2C_SDA 4          // D4 = GPIO5 (piny opisane jako SDA)
#define I2C_SCL 5          // D5 = GPIO6 (piny opisane jako SCL)

// SPI for RC522 RFID (zgodnie z mapowaniem D-pinów)
#define RFID_SS_PIN 2      // D2 = GPIO3 - CS/SDA pin
#define RFID_RST_PIN 1     // D1 = GPIO2 - RST pin
#define RFID_MISO_PIN 9    // D9 = GPIO8 - MISO
#define RFID_MOSI_PIN 10   // D10 = GPIO9 - MOSI
#define RFID_SCK_PIN 8     // D8 = GPIO7 - SCK

// Lock Control
#define LOCK_MOSFET_PIN 3  // D3 = GPIO4 - sterowanie MOSFETem

// Buzzer
#define BUZZER_PIN 0  // D0 = GPIO1 - buzzer
#define ENABLE_BUZZER 0

// Vibration Sensor (Waveshare 9536)
#define VIBRATION_SENSOR_PIN 6  // D6 (TX pin) - wejście czujnika wstrząsów
#define ENABLE_VIBRATION_SENSOR 0

// Keypad Configuration (PCF8574) - MOD-01681 12-key keypad
#define PCF8574_ADDRESS 0x20
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3

// Timing Configuration
#define HEARTBEAT_INTERVAL 60000  // Send heartbeat every 60 seconds
#define LOCK_DURATION 5000         // Keep lock open for 5 seconds
#define MQTT_RECONNECT_DELAY 5000  // Wait 5 seconds before MQTT reconnect
#define KEYPAD_DEBOUNCE_MS 200     // Debounce time for keypad (200ms allows fast repeat)
#define RFID_CHECK_INTERVAL_MS 500 // RFID check interval
#define BUZZER_WRONG_PIN_DURATION 1000  // Buzzer beep for 1 second on wrong PIN
#define VIBRATION_DEBOUNCE_MS 200  // Debounce time for vibration sensor

// Access Control
#define MAX_PIN_CODES 50
#define MAX_RFID_CARDS 50
#define PIN_LENGTH 10

#endif // CONFIG_H
