#include "access_control.h"
#include <ctype.h>

AccessControl::AccessControl(RTC_DS3231* rtcInstance) {
    rtc = rtcInstance;
    pinCodeCount = 0;
    rfidCardCount = 0;
    clearPINCodes();
    clearRFIDCards();
    
    // Load saved data from EEPROM
    loadFromEEPROM();
}

// PIN Code Management
bool AccessControl::addPINCode(const char* code, bool isActive, bool hasTimeLimit,
                               DateTime validFrom, DateTime validUntil) {
    if (pinCodeCount >= MAX_PIN_CODES) {
        return false;
    }

    size_t codeLen = strlen(code);
    if (codeLen == 0 || codeLen > PIN_LENGTH) {
        return false;
    }

    for (size_t i = 0; i < codeLen; i++) {
        if (!isdigit(static_cast<unsigned char>(code[i]))) {
            return false;
        }
    }
    
    // Check if code already exists
    for (int i = 0; i < pinCodeCount; i++) {
        if (strcmp(pinCodes[i].code, code) == 0) {
            // Update existing code
            pinCodes[i].isActive = isActive;
            pinCodes[i].hasTimeLimit = hasTimeLimit;
            pinCodes[i].validFrom = validFrom;
            pinCodes[i].validUntil = validUntil;
            saveToEEPROM();
            return true;
        }
    }
    
    // Add new code
    strncpy(pinCodes[pinCodeCount].code, code, PIN_LENGTH);
    pinCodes[pinCodeCount].code[PIN_LENGTH] = '\0';
    pinCodes[pinCodeCount].isActive = isActive;
    pinCodes[pinCodeCount].hasTimeLimit = hasTimeLimit;
    pinCodes[pinCodeCount].validFrom = validFrom;
    pinCodes[pinCodeCount].validUntil = validUntil;
    pinCodeCount++;
    saveToEEPROM();
    return true;
}

bool AccessControl::removePINCode(const char* code) {
    for (int i = 0; i < pinCodeCount; i++) {
        if (strcmp(pinCodes[i].code, code) == 0) {
            // Shift remaining codes
            for (int j = i; j < pinCodeCount - 1; j++) {
                pinCodes[j] = pinCodes[j + 1];
            }
            pinCodeCount--;
            saveToEEPROM();
            return true;
        }
    }
    return false;
}

bool AccessControl::validatePIN(const char* code) {
    for (int i = 0; i < pinCodeCount; i++) {
        if (strcmp(pinCodes[i].code, code) == 0 && pinCodes[i].isActive) {
            if (pinCodes[i].hasTimeLimit) {
                return isWithinValidTime(pinCodes[i].validFrom, pinCodes[i].validUntil);
            }
            return true;
        }
    }
    return false;
}

void AccessControl::clearPINCodes() {
    pinCodeCount = 0;
    memset(pinCodes, 0, sizeof(pinCodes));
}

int AccessControl::getPINCodeCount() {
    return pinCodeCount;
}

// RFID Card Management
bool AccessControl::addRFIDCard(const char* uid, bool isActive, bool hasTimeLimit,
                                DateTime validFrom, DateTime validUntil) {
    if (rfidCardCount >= MAX_RFID_CARDS) {
        return false;
    }
    
    // Check if card already exists
    for (int i = 0; i < rfidCardCount; i++) {
        if (strcmp(rfidCards[i].uid, uid) == 0) {
            // Update existing card
            rfidCards[i].isActive = isActive;
            rfidCards[i].hasTimeLimit = hasTimeLimit;
            rfidCards[i].validFrom = validFrom;
            rfidCards[i].validUntil = validUntil;
            saveToEEPROM();
            return true;
        }
    }
    
    // Add new card
    strncpy(rfidCards[rfidCardCount].uid, uid, sizeof(rfidCards[rfidCardCount].uid) - 1);
    rfidCards[rfidCardCount].uid[sizeof(rfidCards[rfidCardCount].uid) - 1] = '\0';
    rfidCards[rfidCardCount].isActive = isActive;
    rfidCards[rfidCardCount].hasTimeLimit = hasTimeLimit;
    rfidCards[rfidCardCount].validFrom = validFrom;
    rfidCards[rfidCardCount].validUntil = validUntil;
    rfidCardCount++;
    saveToEEPROM();
    return true;
}

bool AccessControl::removeRFIDCard(const char* uid) {
    for (int i = 0; i < rfidCardCount; i++) {
        if (strcmp(rfidCards[i].uid, uid) == 0) {
            // Shift remaining cards
            for (int j = i; j < rfidCardCount - 1; j++) {
                rfidCards[j] = rfidCards[j + 1];
            }
            rfidCardCount--;
            saveToEEPROM();
            return true;
        }
    }
    return false;
}

bool AccessControl::validateRFID(const char* uid) {
    for (int i = 0; i < rfidCardCount; i++) {
        if (strcmp(rfidCards[i].uid, uid) == 0 && rfidCards[i].isActive) {
            if (rfidCards[i].hasTimeLimit) {
                return isWithinValidTime(rfidCards[i].validFrom, rfidCards[i].validUntil);
            }
            return true;
        }
    }
    return false;
}

void AccessControl::clearRFIDCards() {
    rfidCardCount = 0;
    memset(rfidCards, 0, sizeof(rfidCards));
}

int AccessControl::getRFIDCardCount() {
    return rfidCardCount;
}

// Time-based Validation
bool AccessControl::isWithinValidTime(DateTime validFrom, DateTime validUntil) {
    if (!rtc) {
        return true; // If RTC not available, allow access
    }
    
    DateTime now = rtc->now();
    
    // Check if current time is within valid range
    if (now.unixtime() >= validFrom.unixtime() && now.unixtime() <= validUntil.unixtime()) {
        return true;
    }
    
    return false;
}

// EEPROM Persistence
void AccessControl::saveToEEPROM() {
    preferences.begin("access_ctrl", false);
    
    // Save PIN code count
    preferences.putInt("pin_count", pinCodeCount);
    
    // Save each PIN code (simplified - only code and active status)
    for (int i = 0; i < pinCodeCount; i++) {
        String keyCode = "pin_c_" + String(i);
        String keyActive = "pin_a_" + String(i);
        preferences.putString(keyCode.c_str(), String(pinCodes[i].code));
        preferences.putBool(keyActive.c_str(), pinCodes[i].isActive);
    }
    
    // Save RFID card count
    preferences.putInt("rfid_count", rfidCardCount);
    
    // Save each RFID card
    for (int i = 0; i < rfidCardCount; i++) {
        String keyUID = "rfid_u_" + String(i);
        String keyActive = "rfid_a_" + String(i);
        preferences.putString(keyUID.c_str(), String(rfidCards[i].uid));
        preferences.putBool(keyActive.c_str(), rfidCards[i].isActive);
    }
    
    preferences.end();
    Serial.println("Access codes saved to EEPROM");
}

void AccessControl::loadFromEEPROM() {
    preferences.begin("access_ctrl", true); // Read-only
    
    // Load PIN codes
    int savedPinCount = preferences.getInt("pin_count", 0);
    if (savedPinCount > 0 && savedPinCount <= MAX_PIN_CODES) {
        pinCodeCount = 0;
        for (int i = 0; i < savedPinCount; i++) {
            String keyCode = "pin_c_" + String(i);
            String keyActive = "pin_a_" + String(i);
            
            String code = preferences.getString(keyCode.c_str(), "");
            bool isActive = preferences.getBool(keyActive.c_str(), false);
            
            if (code.length() > 0) {
                strncpy(pinCodes[pinCodeCount].code, code.c_str(), PIN_LENGTH);
                pinCodes[pinCodeCount].code[PIN_LENGTH] = '\0';
                pinCodes[pinCodeCount].isActive = isActive;
                pinCodes[pinCodeCount].hasTimeLimit = false;
                pinCodeCount++;
            }
        }
        Serial.print("Loaded ");
        Serial.print(pinCodeCount);
        Serial.println(" PIN codes from EEPROM");
    }
    
    // Load RFID cards
    int savedRfidCount = preferences.getInt("rfid_count", 0);
    if (savedRfidCount > 0 && savedRfidCount <= MAX_RFID_CARDS) {
        rfidCardCount = 0;
        for (int i = 0; i < savedRfidCount; i++) {
            String keyUID = "rfid_u_" + String(i);
            String keyActive = "rfid_a_" + String(i);
            
            String uid = preferences.getString(keyUID.c_str(), "");
            bool isActive = preferences.getBool(keyActive.c_str(), false);
            
            if (uid.length() > 0) {
                strncpy(rfidCards[rfidCardCount].uid, uid.c_str(), sizeof(rfidCards[rfidCardCount].uid) - 1);
                rfidCards[rfidCardCount].uid[sizeof(rfidCards[rfidCardCount].uid) - 1] = '\0';
                rfidCards[rfidCardCount].isActive = isActive;
                rfidCards[rfidCardCount].hasTimeLimit = false;
                rfidCardCount++;
            }
        }
        Serial.print("Loaded ");
        Serial.print(rfidCardCount);
        Serial.println(" RFID cards from EEPROM");
    }
    
    preferences.end();
}
