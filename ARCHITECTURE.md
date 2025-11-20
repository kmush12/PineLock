# PineLock System Architecture

## Overview

```
┌─────────────────────────────────────────────────────────────┐
│                      Central Server                          │
│                    (Raspberry Pi)                            │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              FastAPI Application                     │   │
│  │  - RESTful API (port 8000)                          │   │
│  │  - SQLite Database                                   │   │
│  │  - Access Management                                 │   │
│  └────────────────┬─────────────────────────────────────┘   │
│                   │                                          │
│  ┌────────────────▼─────────────────────────────────────┐   │
│  │          MQTT Broker (Mosquitto)                     │   │
│  │             - Port 1883                              │   │
│  │             - Topic: pinelock/+/#                    │   │
│  └────────────────┬─────────────────────────────────────┘   │
└───────────────────┼──────────────────────────────────────────┘
                    │
        ┌───────────┴────────────┬────────────────┐
        │                        │                │
        ▼                        ▼                ▼
┌───────────────┐        ┌───────────────┐  ┌───────────────┐
│  Lock Node 1  │        │  Lock Node 2  │  │  Lock Node N  │
│   (ESP32-C3)  │        │   (ESP32-C3)  │  │   (ESP32-C3)  │
├───────────────┤        ├───────────────┤  ├───────────────┤
│ • WiFi        │        │ • WiFi        │  │ • WiFi        │
│ • MQTT Client │        │ • MQTT Client │  │ • MQTT Client │
│ • Keypad      │        │ • Keypad      │  │ • Keypad      │
│ • RFID Reader │        │ • RFID Reader │  │ • RFID Reader │
│ • RTC         │        │ • RTC         │  │ • RTC         │
│ • Lock        │        │ • Lock        │  │ • Lock        │
└───────────────┘        └───────────────┘  └───────────────┘
```

## Communication Flow

### 1. Lock Registration

```
User → API → Database
                  ↓
            Lock Record Created
```

### 2. Access Code/RFID Management

```
User → API → Database
                  ↓
           MQTT Publish → Device
                              ↓
                        Sync Request
                              ↓
                        Update Local Cache
```

### 3. Local Access Attempt

```
User → Keypad/RFID → ESP32 Access Control
                            ↓
                     Check Local Cache
                            ↓
                     Validate (PIN/RFID)
                            ↓
                        ┌───┴───┐
                    Valid    Invalid
                        ↓        ↓
                    Unlock    Deny
                        │        │
                        └────┬───┘
                             ↓
                      MQTT Publish → Server
                                         ↓
                                   Log to Database
```

### 4. Remote Lock Control

```
User → API → MQTT Publish → Device
                                ↓
                          Lock/Unlock
                                ↓
                         Status Update
                                ↓
                          MQTT Publish → Server
                                             ↓
                                      Update Database
```

## Hardware Architecture (Lock Node)

```
┌─────────────────────────────────────────────────────┐
│              Seeed XIAO ESP32-C3                    │
│                                                     │
│  ┌──────────┐  ┌──────────┐  ┌─────────┐          │
│  │   WiFi   │  │   MQTT   │  │ Access  │          │
│  │  Client  │  │  Client  │  │ Control │          │
│  └──────────┘  └──────────┘  └─────────┘          │
│                                                     │
│  GPIO Pins:                                        │
│  • GPIO 6/7  → I2C (SDA/SCL)                      │
│  • GPIO 2/3  → RFID (RST/SS)                      │
│  • GPIO 4/5/8→ SPI (MISO/MOSI/SCK)                │
│  • GPIO 10   → MOSFET Control                     │
└───────┬───────┬────────┬──────────┬────────────────┘
        │       │        │          │
        │       │        │          │
    ┌───▼──┐ ┌──▼───┐ ┌─▼────┐  ┌──▼────┐
    │ PCF  │ │ DS   │ │ RC   │  │ MOSFET│
    │ 8574 │ │ 3231 │ │ 522  │  │ Module│
    └───┬──┘ └──────┘ └──────┘  └───┬───┘
        │                            │
    ┌───▼────┐                   ┌───▼──────┐
    │ 4x4    │                   │ 12V Lock │
    │ Keypad │                   └──────────┘
    └────────┘
```

## Data Flow Diagrams

### PIN Entry Flow

```
┌──────┐      ┌──────────┐      ┌────────────┐      ┌──────┐
│ User │─────▶│ Keypad   │─────▶│ PCF8574    │─────▶│ ESP  │
└──────┘      │ (Matrix) │      │ (I2C)      │      │ C3   │
              └──────────┘      └────────────┘      └───┬──┘
                                                        │
                              ┌─────────────────────────┘
                              │
                              ▼
                    ┌──────────────────┐
                    │ Access Control   │
                    │ • Check PIN      │
                    │ • Check RTC Time │
                    │ • Validate       │
                    └────────┬─────────┘
                             │
                    ┌────────┴────────┐
                 Valid            Invalid
                    │                │
                    ▼                ▼
            ┌──────────────┐   ┌─────────┐
            │ Unlock Lock  │   │  Deny   │
            │ Auto-Relock  │   └─────────┘
            │ Send Event   │
            └──────────────┘
```

### RFID Card Flow

```
┌──────┐      ┌─────────┐      ┌──────────┐
│ Card │─────▶│ RC522   │─────▶│ ESP C3   │
└──────┘      │ (SPI)   │      └────┬─────┘
              └─────────┘           │
                                    ▼
                          ┌──────────────────┐
                          │ Access Control   │
                          │ • Read Card UID  │
                          │ • Check RTC Time │
                          │ • Validate       │
                          └────────┬─────────┘
                                   │
                          ┌────────┴────────┐
                       Valid            Invalid
                          │                │
                          ▼                ▼
                  ┌──────────────┐   ┌─────────┐
                  │ Unlock Lock  │   │  Deny   │
                  │ Auto-Relock  │   └─────────┘
                  │ Send Event   │
                  └──────────────┘
```

## MQTT Topic Structure

```
pinelock/
├── {device_id}/
│   ├── command      (Server → Device)
│   │   └── {"action": "lock|unlock"}
│   │
│   ├── sync         (Server → Device)
│   │   └── {"request": "sync"}
│   │
│   ├── status       (Device → Server)
│   │   └── {"is_locked": bool, "timestamp": int}
│   │
│   ├── access       (Device → Server)
│   │   └── {"access_type": str, "access_method": str, 
│   │        "success": bool, "timestamp": int}
│   │
│   └── heartbeat    (Device → Server)
│       └── {"timestamp": int}
```

## Database Schema

```
┌─────────────────┐
│     locks       │
├─────────────────┤
│ id (PK)         │
│ device_id (UK)  │
│ name            │
│ location        │
│ is_online       │
│ is_locked       │
│ last_seen       │
│ created_at      │
└────────┬────────┘
         │
         │ 1:N
         │
    ┌────┴─────────────────────┬──────────────────┐
    │                          │                  │
┌───▼──────────┐    ┌──────────▼───┐    ┌────────▼──────┐
│ access_codes │    │ rfid_cards   │    │ access_logs   │
├──────────────┤    ├──────────────┤    ├───────────────┤
│ id (PK)      │    │ id (PK)      │    │ id (PK)       │
│ lock_id (FK) │    │ lock_id (FK) │    │ lock_id (FK)  │
│ code         │    │ card_uid     │    │ access_type   │
│ name         │    │ name         │    │ access_method │
│ is_active    │    │ is_active    │    │ success       │
│ valid_from   │    │ valid_from   │    │ timestamp     │
│ valid_until  │    │ valid_until  │    └───────────────┘
│ created_at   │    │ created_at   │
└──────────────┘    └──────────────┘
```

## Security Architecture

```
┌─────────────────────────────────────────────┐
│         Security Layers                     │
├─────────────────────────────────────────────┤
│                                             │
│  1. Network Layer                           │
│     • WiFi WPA2                            │
│     • MQTT Authentication (optional)       │
│     • TLS for MQTT (optional)              │
│                                             │
│  2. Application Layer                       │
│     • API Authentication (future)          │
│     • Access Code Validation               │
│     • RFID Card Validation                 │
│     • Time-based Access Control            │
│                                             │
│  3. Device Layer                            │
│     • Offline Validation                   │
│     • Local Access Control Cache           │
│     • RTC for Time Validation              │
│     • Auto-lock Mechanism                  │
│                                             │
│  4. Audit Layer                             │
│     • Access Logging                       │
│     • Event Reporting                      │
│     • Status Monitoring                    │
│                                             │
└─────────────────────────────────────────────┘
```

## Power States

```
┌──────────────────┐
│  Normal Mode     │  WiFi ON, MQTT Active
│  (~120mA)        │  All sensors active
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  Active Unlock   │  Lock powered
│  (~620mA)        │  5 second duration
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  Normal Mode     │  Return to monitoring
└──────────────────┘
```

## Offline Operation

```
Normal Operation:
┌─────────┐     MQTT      ┌────────┐
│ Server  │◄─────────────►│ Device │
└─────────┘   Connected   └────────┘
                               │
                               ├─ Local Access Control
                               ├─ PIN Validation (RTC)
                               └─ RFID Validation (RTC)

Network Failure:
┌─────────┐               ┌────────┐
│ Server  │    OFFLINE    │ Device │
└─────────┘               └────────┘
                               │
                               ├─ Local Access Control
                               ├─ PIN Validation (RTC)
                               ├─ RFID Validation (RTC)
                               └─ Queue events for sync

Network Restored:
┌─────────┐   Reconnect   ┌────────┐
│ Server  │◄─────────────►│ Device │
└─────────┘               └────────┘
     ▲                         │
     └─────────────────────────┘
          Sync queued events
```
