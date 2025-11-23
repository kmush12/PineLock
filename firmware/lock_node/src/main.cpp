#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MFRC522.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <Adafruit_PCF8574.h>
#include <esp_task_wdt.h>

#include "config.h"
#include "access_control.h"

// Watchdog timeout in seconds
#define WDT_TIMEOUT 30

// WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Hardware instances
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
RTC_DS3231 rtc;
Adafruit_PCF8574 pcf8574;

// Access control
AccessControl accessControl(&rtc);

// Keypad configuration - MOD-01681 (3x4 matrix, 12 keys)
// Column order matches PCF8574 pins: P4(middle), P5(left), P6(right)
const char KEYPAD_KEYS[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'2', '1', '3'},  // Row 1 (P0): middle, left, right
    {'5', '4', '6'},  // Row 2 (P1): middle, left, right
    {'8', '7', '9'},  // Row 3 (P2): middle, left, right
    {'0', '*', '#'}   // Row 4 (P3): middle, left, right
};

// State variables
bool isLocked = true;
unsigned long lastHeartbeat = 0;
unsigned long lockOpenTime = 0;
String currentPIN = "";
char lastKey = '\0';
unsigned long lastKeyTime = 0;
unsigned long buzzerStopTime = 0;
bool buzzerActive = false;
unsigned long lastVibrationTime = 0;

// Function declarations
void setupWiFi();
void setupMQTT();
void setupHardware();
void mqttCallback(char* topic, byte* payload, unsigned int length);
void reconnectMQTT();
void sendHeartbeat();
void sendAccessEvent(const char* accessType, const char* method, bool success);
void sendStatusUpdate();
void sendKeyStatusUpdate(bool keyPresent, String cardUID);
void handleKeypad();
void handleRFID();
void handleVibration();
void controlLock(bool lock);
char readKeypad();
void processPINEntry(char key);
void activateBuzzer(unsigned long duration);
void handleBuzzer();
String getCardUID(MFRC522::Uid* uid);

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== PineLock Firmware ===");
    Serial.println("Device ID: " + String(DEVICE_ID));
    
    // Configure watchdog timer
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    Serial.println("Watchdog configured");
    
    // Initialize hardware
    setupHardware();
    
    // Connect to WiFi
    setupWiFi();
    
    // Setup MQTT
    setupMQTT();
    
    Serial.println("Setup complete!");
}

void loop() {
    // Reset watchdog
    esp_task_wdt_reset();
    
    // Maintain MQTT connection
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();
    
    // Handle keypad input
    handleKeypad();
    
    // Handle RFID key presence detection
    handleRFID();
    
    // Handle vibration detection
    handleVibration();
    
    // Handle buzzer
    handleBuzzer();
    
    // Send periodic heartbeat
    unsigned long currentMillis = millis();
    if (currentMillis - lastHeartbeat > HEARTBEAT_INTERVAL || currentMillis < lastHeartbeat) {
        sendHeartbeat();
        lastHeartbeat = currentMillis;
    }
    
    // Auto-lock after duration (with overflow protection)
    if (!isLocked && lockOpenTime > 0) {
        if (currentMillis - lockOpenTime > LOCK_DURATION || currentMillis < lockOpenTime) {
            controlLock(true);
        }
    }
    
    delay(50);
}

// Hardware flags
bool pcf8574Found = false;
bool rtcFound = false;

void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
        esp_task_wdt_reset(); // Feed watchdog during WiFi connection
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setupMQTT() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
    reconnectMQTT();
}

void setupHardware() {
    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    
    // Initialize PCF8574 for keypad
    if (!pcf8574.begin(PCF8574_ADDRESS, &Wire)) {
        Serial.println("ERROR: PCF8574 not found!");
        pcf8574Found = false;
    } else {
        Serial.println("PCF8574 initialized");
        pcf8574Found = true;
        // Configure all pins as inputs with pullups
        for (int i = 0; i < 8; i++) {
            pcf8574.pinMode(i, INPUT_PULLUP);
        }
    }
    
    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("ERROR: RTC not found!");
        rtcFound = false;
    } else {
        Serial.println("RTC initialized");
        rtcFound = true;
        if (rtc.lostPower()) {
            Serial.println("RTC lost power, setting time to compile time");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
    }
    
    // Initialize RFID with custom SPI pins
    SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
    rfid.PCD_Init();
    Serial.println("RFID initialized");
    
    // Initialize lock control pin
    pinMode(LOCK_MOSFET_PIN, OUTPUT);
    digitalWrite(LOCK_MOSFET_PIN, LOW); // Locked
    
    // Initialize buzzer pin
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Off
    Serial.println("Buzzer initialized");
    
    // Initialize vibration sensor pin
    pinMode(VIBRATION_SENSOR_PIN, INPUT_PULLUP);
    Serial.println("Vibration sensor initialized");
    
    Serial.println("Hardware initialization complete");
    Serial.println("WARNING: No default PIN configured. Add PINs via MQTT.");
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received on topic: ");
    Serial.println(topic);
    
    // Parse topic
    String topicStr = String(topic);
    String prefix = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/";
    
    if (!topicStr.startsWith(prefix)) {
        return;
    }
    
    String messageType = topicStr.substring(prefix.length());
    
    // Parse JSON payload
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }
    
    // Handle different message types
    if (messageType == "command") {
        if (!doc.containsKey("action")) {
            Serial.println("Error: Missing 'action' field in command");
            return;
        }
        
        String action = doc["action"].as<String>();
        Serial.print("Received command: ");
        Serial.println(action);
        
        if (action == "lock") {
            controlLock(true);
            sendAccessEvent("remote", "mqtt", true);
        } else if (action == "unlock") {
            controlLock(false);
            sendAccessEvent("remote", "mqtt", true);
        } else if (action == "add_pin") {
            if (!doc.containsKey("code")) {
                Serial.println("Error: Missing 'code' for add_pin");
                return;
            }

            String code = doc["code"].as<String>();
            bool isActive = doc.containsKey("active") ? doc["active"].as<bool>() : true;

            bool hasTimeLimit = false;
            DateTime validFrom = DateTime();
            DateTime validUntil = DateTime();

            if (doc.containsKey("valid_from") && doc.containsKey("valid_until")) {
                uint32_t fromTs = doc["valid_from"].as<uint32_t>();
                uint32_t untilTs = doc["valid_until"].as<uint32_t>();
                validFrom = DateTime(fromTs);
                validUntil = DateTime(untilTs);
                hasTimeLimit = true;
            }

            bool added = accessControl.addPINCode(code.c_str(), isActive, hasTimeLimit, validFrom, validUntil);
            Serial.println(added ? "PIN added via MQTT" : "Failed to add PIN via MQTT");
            sendAccessEvent("admin_pin_add", code.c_str(), added);
            if (added) {
                sendStatusUpdate();
            }
        } else if (action == "remove_pin") {
            if (!doc.containsKey("code")) {
                Serial.println("Error: Missing 'code' for remove_pin");
                return;
            }

            String code = doc["code"].as<String>();
            bool removed = accessControl.removePINCode(code.c_str());
            Serial.println(removed ? "PIN removed via MQTT" : "Failed to remove PIN via MQTT");
            sendAccessEvent("admin_pin_remove", code.c_str(), removed);
            if (removed) {
                sendStatusUpdate();
            }
        } else {
            Serial.println("Unknown command action: " + action);
        }
    } else if (messageType == "config") {
        Serial.println("Config update received from server");
        handleConfigUpdate(doc);
    } else if (messageType == "sync") {
        Serial.println("Sync request received (not yet implemented)");
        // TODO: Request access codes and RFID cards from server
    }
}

void reconnectMQTT() {
    while (!mqttClient.connected()) {
        esp_task_wdt_reset(); // Feed the watchdog!
        Serial.print("Attempting MQTT connection...");
        
        String clientId = "PineLock-" + String(DEVICE_ID);
        
        // Set keepalive before connecting
        mqttClient.setKeepAlive(120); // 2 minutes keepalive
        
        bool connected;
        if (strlen(MQTT_USERNAME) > 0) {
            connected = mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
        } else {
            connected = mqttClient.connect(clientId.c_str());
        }
        
        if (connected) {
            Serial.println("connected!");
            
            // Subscribe to command, config, and sync topics
            String commandTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/command";
            String configTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/config";
            String syncTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/sync";
            
            mqttClient.subscribe(commandTopic.c_str());
            mqttClient.subscribe(configTopic.c_str());
            mqttClient.subscribe(syncTopic.c_str());
            
            Serial.println("Subscribed to topics");
            
            // Send initial status
            sendStatusUpdate();
            requestConfigSync();
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" retrying in 5 seconds");
            // Wait 5 seconds but keep feeding watchdog
            unsigned long startWait = millis();
            while (millis() - startWait < MQTT_RECONNECT_DELAY) {
                esp_task_wdt_reset();
                delay(100);
            }
        }
    }
}

void sendHeartbeat() {
    if (!mqttClient.connected()) {
        return;
    }
    
    String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/heartbeat";
    StaticJsonDocument<128> doc;
    
    if (rtcFound) {
        doc["timestamp"] = rtc.now().unixtime();
    } else {
        doc["timestamp"] = millis() / 1000; // Fallback to uptime
    }
    
    char buffer[128];
    serializeJson(doc, buffer);
    
    mqttClient.publish(topic.c_str(), buffer);
    Serial.println("Heartbeat sent");
}

void sendAccessEvent(const char* accessType, const char* method, bool success) {
    if (!mqttClient.connected()) {
        return;
    }
    
    String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/access";
    StaticJsonDocument<256> doc;
    doc["access_type"] = accessType;
    doc["access_method"] = method;
    doc["success"] = success;
    
    if (rtcFound) {
        doc["timestamp"] = rtc.now().unixtime();
    } else {
        doc["timestamp"] = millis() / 1000;
    }
    
    char buffer[256];
    serializeJson(doc, buffer);
    
    mqttClient.publish(topic.c_str(), buffer);
    Serial.print("Access event sent: ");
    Serial.println(buffer);
}

void sendStatusUpdate() {
    if (!mqttClient.connected()) {
        return;
    }

    String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/status";
    StaticJsonDocument<128> doc;
    doc["is_locked"] = isLocked;
    doc["is_key_present"] = false; // Will be updated by RFID detection
    
    if (rtcFound) {
        doc["timestamp"] = rtc.now().unixtime();
    } else {
        doc["timestamp"] = millis() / 1000;
    }

    char buffer[128];
    serializeJson(doc, buffer);

    mqttClient.publish(topic.c_str(), buffer);
    Serial.println("Status update sent");
}

void handleConfigUpdate(const JsonDocument& doc) {
    Serial.println("Applying configuration update...");

    // Update PIN codes
    accessControl.clearPINCodes();
    if (doc.containsKey("access_codes") && doc["access_codes"].is<JsonArray>()) {
        JsonArrayConst pins = doc["access_codes"].as<JsonArrayConst>();
        for (JsonVariantConst pinValue : pins) {
            const char* code = pinValue.as<const char*>();
            if (code && strlen(code) > 0) {
                accessControl.addPINCode(code);
            }
        }
    }
    Serial.print("Loaded PIN codes: ");
    Serial.println(accessControl.getPINCodeCount());

    // Update RFID cards
    accessControl.clearRFIDCards();
    if (doc.containsKey("rfid_cards") && doc["rfid_cards"].is<JsonArray>()) {
        JsonArrayConst cards = doc["rfid_cards"].as<JsonArrayConst>();
        for (JsonVariantConst cardValue : cards) {
            const char* uid = cardValue.as<const char*>();
            if (uid && strlen(uid) > 0) {
                accessControl.addRFIDCard(uid);
            }
        }
    }
    Serial.print("Loaded RFID cards: ");
    Serial.println(accessControl.getRFIDCardCount());

    if (doc.containsKey("key_tag")) {
        const char* keyTag = doc["key_tag"].as<const char*>();
        if (keyTag && strlen(keyTag) > 0) {
            Serial.print("Key tag configured: ");
            Serial.println(keyTag);
        } else {
            Serial.println("Key tag cleared or not provided");
        }
    } else {
        Serial.println("Key tag not included in config");
    }

    sendStatusUpdate();
}

void requestConfigSync() {
    if (!mqttClient.connected()) {
        Serial.println("Cannot request config sync - MQTT disconnected");
        return;
    }
    StaticJsonDocument<64> doc;
    doc["request"] = "config";
    String payload;
    serializeJson(doc, payload);
    String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/sync";
    bool published = mqttClient.publish(topic.c_str(), payload.c_str());
    Serial.println(published ? "Sync request sent" : "Failed to send sync request");
}

void sendKeyStatusUpdate(bool keyPresent, String cardUID) {
    if (!mqttClient.connected()) {
        return;
    }

    String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/status";
    StaticJsonDocument<256> doc;
    doc["is_locked"] = isLocked;
    doc["is_key_present"] = keyPresent;
    if (keyPresent && cardUID.length() > 0) {
        doc["key_uid"] = cardUID;
    }
    
    if (rtcFound) {
        doc["timestamp"] = rtc.now().unixtime();
    } else {
        doc["timestamp"] = millis() / 1000;
    }

    char buffer[256];
    serializeJson(doc, buffer);

    mqttClient.publish(topic.c_str(), buffer);
    Serial.print("Key status update sent: ");
    Serial.println(keyPresent ? "present" : "absent");
}

void handleKeypad() {
    char key = readKeypad();
    unsigned long currentMillis = millis();
    
    if (key != '\0' && key != lastKey) {
        Serial.print("Key pressed: ");
        Serial.println(key);
        lastKey = key;
        lastKeyTime = currentMillis;
        
        processPINEntry(key);
    }
    
    // Clear last key after timeout to allow repeat presses (with overflow protection)
    if (currentMillis - lastKeyTime > KEYPAD_DEBOUNCE_MS || currentMillis < lastKeyTime) {
        lastKey = '\0';
    }
}

char readKeypad() {
    if (!pcf8574Found) return '\0';

    // Scan keypad matrix through PCF8574
    // PCF8574 pins: P0-P3 = Rows, P4-P7 = Columns
    
    for (int col = 0; col < KEYPAD_COLS; col++) {
        // Set all columns high (via pullups)
        for (int c = 0; c < KEYPAD_COLS; c++) {
            pcf8574.digitalWrite(c + 4, HIGH);
        }
        
        // Set current column low
        pcf8574.digitalWrite(col + 4, LOW);
        
        delay(5); // Small delay for signal stabilization
        
        // Check all rows
        for (int row = 0; row < KEYPAD_ROWS; row++) {
            if (pcf8574.digitalRead(row) == LOW) {
                // Key pressed at this row/col
                return KEYPAD_KEYS[row][col];
            }
        }
    }
    
    return '\0'; // No key pressed
}

void processPINEntry(char key) {
    if (key == '#') {
        // Submit PIN
        if (currentPIN.length() > 0) {
            Serial.print("Validating PIN: ");
            Serial.println(currentPIN);
            
            bool valid = accessControl.validatePIN(currentPIN.c_str());
            
            if (valid) {
                Serial.println("PIN valid! Unlocking...");
                controlLock(false);
                sendAccessEvent("pin", currentPIN.c_str(), true);
            } else {
                Serial.println("PIN invalid!");
                activateBuzzer(BUZZER_WRONG_PIN_DURATION);
                sendAccessEvent("pin", currentPIN.c_str(), false);
            }
            
            currentPIN = "";
        }
    } else if (key == '*') {
        // Clear PIN
        currentPIN = "";
        Serial.println("PIN cleared");
    } else if (key >= '0' && key <= '9') {
        // Add digit to PIN up to configured max length
        if (currentPIN.length() < PIN_LENGTH) {
            currentPIN += key;
        }
    }
}

void handleRFID() {
    static bool lastKeyPresent = false;
    static unsigned long lastRFIDCheck = 0;
    unsigned long currentMillis = millis();

    // Check RFID presence every 500ms to avoid spam (with overflow protection)
    if (currentMillis - lastRFIDCheck < RFID_CHECK_INTERVAL_MS && currentMillis >= lastRFIDCheck) {
        return;
    }
    lastRFIDCheck = currentMillis;

    // Check if a card is present
    bool keyPresent = rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial();

    // Only report changes in key presence
    if (keyPresent != lastKeyPresent) {
        lastKeyPresent = keyPresent;

        if (keyPresent) {
            String cardUID = getCardUID(&rfid.uid);
            Serial.print("RFID key detected in box: ");
            Serial.println(cardUID);

            // Check if this is a registered key
            bool valid = accessControl.validateRFID(cardUID.c_str());
            if (valid) {
                Serial.println("Valid key present in box");
                sendKeyStatusUpdate(true, cardUID.c_str());
            } else {
                Serial.println("Unknown key detected in box");
                sendKeyStatusUpdate(true, cardUID.c_str());
            }

            // Halt PICC
            rfid.PICC_HaltA();
            rfid.PCD_StopCrypto1();
        } else {
            Serial.println("RFID key removed from box");
            sendKeyStatusUpdate(false, "");
        }
    }
}

String getCardUID(MFRC522::Uid* uid) {
    String uidStr = "";
    for (byte i = 0; i < uid->size; i++) {
        if (uid->uidByte[i] < 0x10) {
            uidStr += "0";
        }
        uidStr += String(uid->uidByte[i], HEX);
    }
    uidStr.toUpperCase();
    return uidStr;
}

void controlLock(bool lock) {
    isLocked = lock;
    
    if (lock) {
        digitalWrite(LOCK_MOSFET_PIN, LOW);
        Serial.println("Lock LOCKED");
        lockOpenTime = 0;
    } else {
        digitalWrite(LOCK_MOSFET_PIN, HIGH);
        Serial.println("Lock UNLOCKED");
        lockOpenTime = millis();
    }
    
    sendStatusUpdate();
}

void activateBuzzer(unsigned long duration) {
    buzzerActive = true;
    buzzerStopTime = millis() + duration;
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("Buzzer activated");
}

void handleBuzzer() {
    if (buzzerActive) {
        unsigned long currentMillis = millis();
        if (currentMillis >= buzzerStopTime || currentMillis < buzzerStopTime - BUZZER_WRONG_PIN_DURATION - 1000) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerActive = false;
            Serial.println("Buzzer deactivated");
        }
    }
}

void handleVibration() {
    static bool lastVibrationState = HIGH;
    unsigned long currentMillis = millis();
    
    int vibrationState = digitalRead(VIBRATION_SENSOR_PIN);
    
    // Detect falling edge (vibration detected)
    if (vibrationState == LOW && lastVibrationState == HIGH) {
        // Debounce check
        if (currentMillis - lastVibrationTime > VIBRATION_DEBOUNCE_MS || currentMillis < lastVibrationTime) {
            lastVibrationTime = currentMillis;
            
            Serial.println("VIBRATION DETECTED!");
            
            // Activate buzzer
            activateBuzzer(BUZZER_WRONG_PIN_DURATION);
            
            // Send vibration alert via MQTT
            String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/alert";
            StaticJsonDocument<128> doc;
            doc["type"] = "vibration";
            doc["timestamp"] = rtc.now().unixtime();
            
            String jsonString;
            serializeJson(doc, jsonString);
            mqttClient.publish(topic.c_str(), jsonString.c_str());
            
            Serial.println("Vibration alert sent");
        }
    }
    
    lastVibrationState = vibrationState;
}
