#ifndef ACCESS_CONTROL_H
#define ACCESS_CONTROL_H

#include <Arduino.h>
#include <RTClib.h>
#include <Preferences.h>
#include "config.h"

// Structure for PIN codes
struct PINCode {
    char code[PIN_LENGTH + 1];
    bool isActive;
    DateTime validFrom;
    DateTime validUntil;
    bool hasTimeLimit;
};

// Structure for RFID cards (used for key presence detection)
struct RFIDCard {
    char uid[20];
    bool isActive;
    DateTime validFrom;
    DateTime validUntil;
    bool hasTimeLimit;
};

class AccessControl {
private:
    PINCode pinCodes[MAX_PIN_CODES];
    RFIDCard rfidCards[MAX_RFID_CARDS];
    int pinCodeCount;
    int rfidCardCount;
    RTC_DS3231* rtc;
    Preferences preferences;

public:
    AccessControl(RTC_DS3231* rtcInstance);
    void begin();
    
    // PIN code management
    bool addPINCode(const char* code, bool isActive = true, bool hasTimeLimit = false,
                    DateTime validFrom = DateTime(), DateTime validUntil = DateTime());
    bool removePINCode(const char* code);
    bool validatePIN(const char* code);
    void clearPINCodes();
    int getPINCodeCount();
    
    // RFID card management (for key presence detection)
    bool addRFIDCard(const char* uid, bool isActive = true, bool hasTimeLimit = false,
                      DateTime validFrom = DateTime(), DateTime validUntil = DateTime());
    bool removeRFIDCard(const char* uid);
    bool validateRFID(const char* uid);  // Validates if card is registered for this lock
    void clearRFIDCards();
    int getRFIDCardCount();
    
    // Time-based validation
    bool isWithinValidTime(DateTime validFrom, DateTime validUntil);
    
    // Persistence
    void saveToEEPROM();
    void loadFromEEPROM();
};

#endif // ACCESS_CONTROL_H
