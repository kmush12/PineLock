#include "access_control.h"

AccessControl::AccessControl(RTC_DS3231* rtcInstance) {
    rtc = rtcInstance;
    pinCodeCount = 0;
    rfidCardCount = 0;
    clearPINCodes();
    clearRFIDCards();
}

// PIN Code Management
bool AccessControl::addPINCode(const char* code, bool isActive, bool hasTimeLimit,
                               DateTime validFrom, DateTime validUntil) {
    if (pinCodeCount >= MAX_PIN_CODES) {
        return false;
    }
    
    // Check if code already exists
    for (int i = 0; i < pinCodeCount; i++) {
        if (strcmp(pinCodes[i].code, code) == 0) {
            // Update existing code
            pinCodes[i].isActive = isActive;
            pinCodes[i].hasTimeLimit = hasTimeLimit;
            pinCodes[i].validFrom = validFrom;
            pinCodes[i].validUntil = validUntil;
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
