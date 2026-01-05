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
String keyTagUID = "";  // UID of the key tag for presence detection

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
void handleDoorSensor();
void controlLock(bool lock);
char readKeypad();
void processPINEntry(char key);
void activateBuzzer(unsigned long duration);
void handleBuzzer();
String getCardUID(MFRC522::Uid* uid);

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== PineLock Firmware ===");
    Serial.println("Device ID: " + String(DEVICE_ID));
    Serial.print("Firmware built: ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);
    
    // Configure watchdog timer
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    Serial.println("Watchdog configured");
    
    // Initialize hardware first
    setupHardware();
    
    // Initialize access control (NVS)
    accessControl.begin();
    
    // Start WiFi connection (non-blocking)
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
    
    // Handle door sensor
    handleDoorSensor();
    
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
    
    // Non-blocking: we just start the connection and check status later
    Serial.println("WiFi connection started...");
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
    
    // Test RFID communication
    byte version = rfid.PCD_ReadRegister(rfid.VersionReg);
    Serial.print("RFID initialized - Version: 0x");
    Serial.println(version, HEX);
    if (version == 0x00 || version == 0xFF) {
        Serial.println("WARNING: RC522 communication failed! Check wiring.");
    } else {
        Serial.println("RC522 communication OK");
    }
    
    // Initialize lock control pin (GPIO2 has pull-down during boot - safe from glitches)
    pinMode(LOCK_MOSFET_PIN, OUTPUT);
    digitalWrite(LOCK_MOSFET_PIN, LOW); // Locked
    
    // Initialize buzzer pin
#if ENABLE_BUZZER
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW); // Off
    Serial.println("Buzzer initialized");
#else
    Serial.println("Buzzer disabled (ENABLE_BUZZER=0)");
#endif
    
    // Initialize vibration sensor pin
#if ENABLE_VIBRATION_SENSOR
    pinMode(VIBRATION_SENSOR_PIN, INPUT_PULLUP);
    Serial.println("Vibration sensor initialized");
#else
    Serial.println("Vibration sensor disabled (ENABLE_VIBRATION_SENSOR=0)");
#endif

    // Initialize door sensor pin
#if ENABLE_DOOR_SENSOR
    pinMode(DOOR_SENSOR_PIN, INPUT_PULLUP);
    Serial.println("Door sensor initialized");
#else
    Serial.println("Door sensor disabled (ENABLE_DOOR_SENSOR=0)");
#endif
    
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
    StaticJsonDocument<256> doc;
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
        } else if (action == "buzzer") {
            activateBuzzer(2000);
            Serial.println("Buzzer triggered via MQTT");
        } else {
            Serial.println("Unknown command action: " + action);
        }
    } else if (messageType == "sync") {
        Serial.println("Sync request received - waiting for config data");
    } else if (messageType == "config") {
        Serial.println("Config received from server");
        
        // Clear all existing PINs and RFID cards
        accessControl.clearPINCodes();
        accessControl.clearRFIDCards();
        
        // Process access_codes array
        if (doc.containsKey("access_codes")) {
            JsonArray codes = doc["access_codes"].as<JsonArray>();
            Serial.print("Received ");
            Serial.print(codes.size());
            Serial.println(" PIN codes");
            
            for (JsonVariant codeVariant : codes) {
                String code = codeVariant.as<String>();
                bool added = accessControl.addPINCode(code.c_str(), true, false, DateTime(), DateTime());
                Serial.print("  PIN ");
                Serial.print(code);
                Serial.println(added ? " - added" : " - failed");
            }
        }
        
        // Process rfid_cards array
        if (doc.containsKey("rfid_cards")) {
            JsonArray cards = doc["rfid_cards"].as<JsonArray>();
            Serial.print("Received ");
            Serial.print(cards.size());
            Serial.println(" RFID cards");
            
            for (JsonVariant cardVariant : cards) {
                String cardUid = cardVariant.as<String>();
                bool added = accessControl.addRFIDCard(cardUid.c_str(), true, false, DateTime(), DateTime());
                Serial.print("  Card ");
                Serial.print(cardUid);
                Serial.println(added ? " - added" : " - failed");
            }
        }
        
        // Process key_tag
        if (doc.containsKey("key_tag") && !doc["key_tag"].isNull()) {
            keyTagUID = doc["key_tag"].as<String>();
            Serial.print("Key tag UID configured: ");
            Serial.println(keyTagUID);
        } else {
            keyTagUID = "";
            Serial.println("No key tag configured");
        }
        
        Serial.println("Config sync completed!");
    }
}

void reconnectMQTT() {
    // Loop until we're reconnected
    while (!mqttClient.connected()) {
        esp_task_wdt_reset(); // Feed the watchdog!
        
        // Check WiFi status first
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi not connected, waiting...");
            delay(1000);
            continue;
        }

        Serial.print("Attempting MQTT connection to ");
        Serial.print(MQTT_BROKER);
        Serial.print(":");
        Serial.print(MQTT_PORT);
        Serial.print("...");
        
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
            
            // Subscribe to command, sync, and config topics
            String commandTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/command";
            String syncTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/sync";
            String configTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/config";
            
            mqttClient.subscribe(commandTopic.c_str());
            mqttClient.subscribe(syncTopic.c_str());
            mqttClient.subscribe(configTopic.c_str());
            
            Serial.println("Subscribed to topics");
            
            // Send initial status
            sendStatusUpdate();
            
            // Request sync
            String syncRequestTopic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/sync";
            StaticJsonDocument<64> syncDoc;
            syncDoc["request"] = "sync";
            char syncBuffer[64];
            serializeJson(syncDoc, syncBuffer);
            mqttClient.publish(syncRequestTopic.c_str(), syncBuffer);
            Serial.println("Sync requested");
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
    
    // Process key if:
    // 1. Valid key pressed ('\0' = no key)
    // 2. Different from last key OR debounce timeout expired (allows same key repeat)
    if (key != '\0') {
        bool isDifferentKey = (key != lastKey);
        bool debounceExpired = (currentMillis - lastKeyTime > KEYPAD_DEBOUNCE_MS || currentMillis < lastKeyTime);
        
        if (isDifferentKey || debounceExpired) {
            Serial.print("Key pressed: ");
            Serial.println(key);
            lastKey = key;
            lastKeyTime = currentMillis;
            
            processPINEntry(key);
        }
    } else {
        // No key pressed - clear last key immediately to allow fast repeat
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
            Serial.print(currentPIN);
            Serial.print(" (length: ");
            Serial.print(currentPIN.length());
            Serial.println(")");
            
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
        Serial.print("PIN cleared (was: ");
        Serial.print(currentPIN.length());
        Serial.println(" digits)");
        currentPIN = "";
    } else if (key >= '0' && key <= '9') {
        // Add digit to PIN up to configured max length
        if (currentPIN.length() < PIN_LENGTH) {
            currentPIN += key;
            Serial.print("PIN digit added: ");
            Serial.print(key);
            Serial.print(" (total: ");
            Serial.print(currentPIN.length());
            Serial.println(" digits)");
        } else {
            Serial.println("PIN max length reached, ignoring digit");
        }
    }
}

void handleRFID() {
    static bool lastKeyPresent = false;
    static unsigned long lastRFIDCheck = 0;
    static unsigned long debugPrintTime = 0;
    unsigned long currentMillis = millis();

    // Check RFID presence every 500ms to avoid spam (with overflow protection)
    if (currentMillis - lastRFIDCheck < RFID_CHECK_INTERVAL_MS && currentMillis >= lastRFIDCheck) {
        return;
    }
    lastRFIDCheck = currentMillis;

    // Debug print every 10 seconds
    if (currentMillis - debugPrintTime > 10000) {
        debugPrintTime = currentMillis;
        Serial.println("[RFID Debug] Checking for cards...");
    }

    // Check if a card is present
    bool cardPresent = rfid.PICC_IsNewCardPresent();
    
    // If no new card, try to wake up halted cards (for continuous presence)
    if (!cardPresent) {
        byte bufferATQA[2];
        byte bufferSize = sizeof(bufferATQA);
        MFRC522::StatusCode status = rfid.PICC_WakeupA(bufferATQA, &bufferSize);
        if (status == MFRC522::STATUS_OK || status == MFRC522::STATUS_COLLISION) {
            cardPresent = true;
        } else {
            // Serial.print("[RFID Debug] Wakeup failed: ");
            // Serial.println(rfid.GetStatusCodeName(status));
        }
    }

    bool cardRead = false;
    String currentCardUID = "";
    
    if (cardPresent) {
        cardRead = rfid.PICC_ReadCardSerial();
        if (cardRead) {
            currentCardUID = getCardUID(&rfid.uid);
        } else {
            Serial.println("[RFID Debug] Failed to read card serial");
        }
    }
    
    bool keyPresent = cardPresent && cardRead;
    static unsigned long lastSeenTime = 0;

    if (keyPresent) {
        lastSeenTime = currentMillis;
    }

    // Debounce removal: Only consider removed if not seen for 3 seconds
    bool effectiveKeyPresent = keyPresent;
    if (!keyPresent && lastKeyPresent && (currentMillis - lastSeenTime < 3000)) {
        effectiveKeyPresent = true; // Keep it present during grace period
        // Serial.println("[RFID Debug] Grace period active");
    }

    // Only report changes in effective key presence
    if (effectiveKeyPresent != lastKeyPresent) {
        lastKeyPresent = effectiveKeyPresent;

        if (effectiveKeyPresent) {
            // If we are here, it means we either just found a key, or we recovered from a glitch
            // We should use the UID we just read. If we are in grace period (keyPresent=false), 
            // we might not have a UID this cycle, but we shouldn't be toggling state anyway.
            // Actually, if effectiveKeyPresent becomes true, it must be because keyPresent is true.
            
            Serial.print("RFID card detected: ");
            Serial.println(currentCardUID);

            // Check if this is the key tag
            if (keyTagUID.length() > 0 && currentCardUID == keyTagUID) {
                Serial.println("Key tag detected - presence confirmed");
                sendKeyStatusUpdate(true, currentCardUID.c_str());
            } else {
                // Check if this is a registered access card
                bool valid = accessControl.validateRFID(currentCardUID.c_str());
                if (valid) {
                    Serial.println("Valid access card - unlocking");
                    controlLock(false);
                    sendAccessEvent("rfid", currentCardUID.c_str(), true);
                } else {
                    Serial.println("Unknown RFID card");
                    sendAccessEvent("rfid", currentCardUID.c_str(), false);
                }
            }

            // Halt PICC
            rfid.PICC_HaltA();
        } else {
            Serial.println("RFID key removed from box");
            sendKeyStatusUpdate(false, "");
        }
    } else if (effectiveKeyPresent && keyPresent) {
        // Still present, just Halt to allow next detection
        rfid.PICC_HaltA();
    }
    
    // Recovery: If no card found, check if module is still responsive
    if (!cardPresent && (currentMillis - lastRFIDCheck > 2000)) {
        byte v = rfid.PCD_ReadRegister(rfid.VersionReg);
        if (v == 0x00 || v == 0xFF) {
            Serial.println("[RFID] Module unresponsive (Version 0x00/0xFF), re-initializing...");
            rfid.PCD_Init();
        }
    }
}

String getCardUID(MFRC522::Uid* uid) {
    String uidStr = "";
    for (byte i = 0; i < uid->size; i++) {
        if (i > 0) uidStr += ":";
        if (uid->uidByte[i] < 0x10) {
            uidStr += "0";
        }
        uidStr += String(uid->uidByte[i], HEX);
    }
    uidStr.toLowerCase();
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
#if !ENABLE_BUZZER
    (void)duration;
    return;
#else
    buzzerActive = true;
    buzzerStopTime = millis() + duration;
    digitalWrite(BUZZER_PIN, HIGH);
    Serial.println("Buzzer activated");
#endif
}

void handleBuzzer() {
#if !ENABLE_BUZZER
    return;
#else
    if (buzzerActive) {
        unsigned long currentMillis = millis();
        if (currentMillis >= buzzerStopTime || currentMillis < buzzerStopTime - BUZZER_WRONG_PIN_DURATION - 1000) {
            digitalWrite(BUZZER_PIN, LOW);
            buzzerActive = false;
            Serial.println("Buzzer deactivated");
        }
    }
#endif
}

void handleVibration() {
#if !ENABLE_VIBRATION_SENSOR
    return;
#else
    static bool lastVibrationState = HIGH;
    static int vibrationCount = 0;
    static unsigned long firstVibrationTime = 0;
    unsigned long currentMillis = millis();
    
    int vibrationState = digitalRead(VIBRATION_SENSOR_PIN);
    
    // Reset count if window expired
    if (vibrationCount > 0 && (currentMillis - firstVibrationTime > VIBRATION_WINDOW_MS)) {
        Serial.println("Vibration window expired, resetting count");
        vibrationCount = 0;
    }
    
    // Detect falling edge (vibration detected)
    if (vibrationState == LOW && lastVibrationState == HIGH) {
        // Debounce check
        if (currentMillis - lastVibrationTime > VIBRATION_DEBOUNCE_MS || currentMillis < lastVibrationTime) {
            lastVibrationTime = currentMillis;
            
            // Increment count
            vibrationCount++;
            if (vibrationCount == 1) {
                firstVibrationTime = currentMillis;
            }
            
            Serial.print("VIBRATION DETECTED! Count: ");
            Serial.print(vibrationCount);
            Serial.print("/");
            Serial.println(VIBRATION_THRESHOLD_COUNT);
            
            // Check threshold
            if (vibrationCount >= VIBRATION_THRESHOLD_COUNT) {
                Serial.println("ALARM TRIGGERED!");
                
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
                
                // Reset count after alarm
                vibrationCount = 0;
            }
        }
    }
    
    lastVibrationState = vibrationState;
#endif
}

void handleDoorSensor() {
#if !ENABLE_DOOR_SENSOR
    return;
#else
    static bool lastDoorState = HIGH; // HIGH = Open (pullup), LOW = Closed (magnet)
    static unsigned long lastDoorStateChange = 0;
    static bool alertSent = false;
    unsigned long currentMillis = millis();
    
    int doorState = digitalRead(DOOR_SENSOR_PIN);
    
    // Debounce (simple 50ms check)
    if (doorState != lastDoorState) {
        delay(50); // Simple blocking debounce for state change
        if (digitalRead(DOOR_SENSOR_PIN) == doorState) {
            lastDoorState = doorState;
            lastDoorStateChange = currentMillis;
            alertSent = false;
            
            bool isOpen = (doorState == HIGH);
            Serial.print("Door state changed: ");
            Serial.println(isOpen ? "OPEN" : "CLOSED");
            
            // Send status update
            String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/status";
            StaticJsonDocument<128> doc;
            doc["is_locked"] = isLocked;
            doc["is_door_open"] = isOpen;
            doc["timestamp"] = rtc.now().unixtime();
            
            String jsonString;
            serializeJson(doc, jsonString);
            mqttClient.publish(topic.c_str(), jsonString.c_str());
        }
    }
    
    // Check for open door alert
    if (doorState == HIGH && !alertSent) {
        if (currentMillis - lastDoorStateChange > DOOR_OPEN_ALERT_MS) {
            Serial.println("ALARM: Door open too long!");
            
            // Send alert
            String topic = String(MQTT_TOPIC_PREFIX) + "/" + String(DEVICE_ID) + "/alert";
            StaticJsonDocument<128> doc;
            doc["type"] = "door_open_alert";
            doc["message"] = "Door has been open for more than 3 minutes";
            doc["timestamp"] = rtc.now().unixtime();
            
            String jsonString;
            serializeJson(doc, jsonString);
            mqttClient.publish(topic.c_str(), jsonString.c_str());
            
            alertSent = true;
            
            // Optional: beep buzzer once
            activateBuzzer(200);
        }
    }
#endif
}
