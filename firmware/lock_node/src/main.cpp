#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MFRC522.h>
#include <RTClib.h>
#include <ArduinoJson.h>
#include <Adafruit_PCF8574.h>

#include "config.h"
#include "access_control.h"

// WiFi and MQTT clients
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Hardware instances
MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);
RTC_DS3231 rtc;
Adafruit_PCF8574 pcf8574;

// Access control
AccessControl* accessControl;

// Keypad configuration
const char KEYPAD_KEYS[KEYPAD_ROWS][KEYPAD_COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// State variables
bool isLocked = true;
unsigned long lastHeartbeat = 0;
unsigned long lockOpenTime = 0;
String currentPIN = "";
char lastKey = '\0';
unsigned long lastKeyTime = 0;

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
void controlLock(bool lock);
char readKeypad();
void processPINEntry(char key);
String getCardUID(MFRC522::Uid* uid);

void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== PineLock Firmware ===");
    Serial.println("Device ID: " + String(DEVICE_ID));
    
    // Initialize hardware
    setupHardware();
    
    // Connect to WiFi
    setupWiFi();
    
    // Setup MQTT
    setupMQTT();
    
    Serial.println("Setup complete!");
}

void loop() {
    // Maintain MQTT connection
    if (!mqttClient.connected()) {
        reconnectMQTT();
    }
    mqttClient.loop();
    
    // Handle keypad input
    handleKeypad();
    
    // Handle RFID key presence detection
    handleRFID();
    
    // Send periodic heartbeat
    if (millis() - lastHeartbeat > HEARTBEAT_INTERVAL) {
        sendHeartbeat();
        lastHeartbeat = millis();
    }
    
    // Auto-lock after duration
    if (!isLocked && lockOpenTime > 0 && millis() - lockOpenTime > LOCK_DURATION) {
        controlLock(true);
    }
    
    delay(50);
}

void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(WIFI_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while (WiFi.status() != WL_CONNECTED) {
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
    } else {
        Serial.println("PCF8574 initialized");
        // Configure all pins as inputs with pullups
        for (int i = 0; i < 8; i++) {
            pcf8574.pinMode(i, INPUT_PULLUP);
        }
    }
    
    // Initialize RTC
    if (!rtc.begin()) {
        Serial.println("ERROR: RTC not found!");
    } else {
        Serial.println("RTC initialized");
        if (rtc.lostPower()) {
            Serial.println("RTC lost power, setting time to compile time");
            rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        }
    }
    
    // Initialize RFID
    SPI.begin();
    rfid.PCD_Init();
    Serial.println("RFID initialized");
    
    // Initialize lock control pin
    pinMode(LOCK_MOSFET_PIN, OUTPUT);
    digitalWrite(LOCK_MOSFET_PIN, LOW); // Locked
    
    // Initialize access control
    accessControl = new AccessControl(&rtc);
    
    // Add default PIN for testing (remove in production)
    accessControl->addPINCode("1234", true, false, DateTime(), DateTime());
    
    Serial.println("Hardware initialization complete");
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
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    
    if (error) {
        Serial.print("JSON parse error: ");
        Serial.println(error.c_str());
        return;
    }
    
    // Handle different message types
    if (messageType == "command") {
        String action = doc["action"];
        Serial.print("Received command: ");
        Serial.println(action);
        
        if (action == "lock") {
            controlLock(true);
            sendAccessEvent("remote", "mqtt", true);
        } else if (action == "unlock") {
            controlLock(false);
            sendAccessEvent("remote", "mqtt", true);
        }
    } else if (messageType == "sync") {
        Serial.println("Sync request received (not yet implemented)");
        // TODO: Request access codes and RFID cards from server
    }
}

void reconnectMQTT() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        
        String clientId = "PineLock-" + String(DEVICE_ID);
        
        bool connected;
        if (strlen(MQTT_USERNAME) > 0) {
            connected = mqttClient.connect(clientId.c_str(), MQTT_USERNAME, MQTT_PASSWORD);
        } else {
            connected = mqttClient.connect(clientId.c_str());
        }
        
        if (connected) {
            Serial.println("connected!");
            
            // Subscribe to command and sync topics
            String commandTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/command";
            String syncTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/sync";
            
            mqttClient.subscribe(commandTopic.c_str());
            mqttClient.subscribe(syncTopic.c_str());
            
            Serial.println("Subscribed to topics");
            
            // Send initial status
            sendStatusUpdate();
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" retrying in 5 seconds");
            delay(MQTT_RECONNECT_DELAY);
        }
    }
}

void sendHeartbeat() {
    if (!mqttClient.connected()) {
        return;
    }
    
    String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/heartbeat";
    StaticJsonDocument<128> doc;
    doc["timestamp"] = rtc.now().unixtime();
    
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
    doc["timestamp"] = rtc.now().unixtime();
    
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
    doc["timestamp"] = rtc.now().unixtime();

    char buffer[128];
    serializeJson(doc, buffer);

    mqttClient.publish(topic.c_str(), buffer);
    Serial.println("Status update sent");
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
    doc["timestamp"] = rtc.now().unixtime();

    char buffer[256];
    serializeJson(doc, buffer);

    mqttClient.publish(topic.c_str(), buffer);
    Serial.print("Key status update sent: ");
    Serial.println(keyPresent ? "present" : "absent");
}

void handleKeypad() {
    char key = readKeypad();
    
    if (key != '\0' && key != lastKey) {
        Serial.print("Key pressed: ");
        Serial.println(key);
        lastKey = key;
        lastKeyTime = millis();
        
        processPINEntry(key);
    }
    
    // Clear last key after timeout to allow repeat presses
    if (millis() - lastKeyTime > 500) {
        lastKey = '\0';
    }
}

char readKeypad() {
    // Read keypad matrix through PCF8574
    // This is a simplified implementation
    // In a real implementation, you'd scan the matrix properly
    
    // For now, return '\0' to indicate no key pressed
    // TODO: Implement proper matrix scanning
    return '\0';
}

void processPINEntry(char key) {
    if (key == '#') {
        // Submit PIN
        if (currentPIN.length() > 0) {
            Serial.print("Validating PIN: ");
            Serial.println(currentPIN);
            
            bool valid = accessControl->validatePIN(currentPIN.c_str());
            
            if (valid) {
                Serial.println("PIN valid! Unlocking...");
                controlLock(false);
                sendAccessEvent("pin", currentPIN.c_str(), true);
            } else {
                Serial.println("PIN invalid!");
                sendAccessEvent("pin", currentPIN.c_str(), false);
            }
            
            currentPIN = "";
        }
    } else if (key == '*') {
        // Clear PIN
        currentPIN = "";
        Serial.println("PIN cleared");
    } else if (key >= '0' && key <= '9') {
        // Add digit to PIN
        if (currentPIN.length() < 10) {
            currentPIN += key;
        }
    }
}

void handleRFID() {
    static bool lastKeyPresent = false;
    static unsigned long lastRFIDCheck = 0;

    // Check RFID presence every 500ms to avoid spam
    if (millis() - lastRFIDCheck < 500) {
        return;
    }
    lastRFIDCheck = millis();

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
            bool valid = accessControl->validateRFID(cardUID.c_str());
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
