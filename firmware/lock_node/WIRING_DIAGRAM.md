# 🔌 Schemat połączeń PineLock - Seeed XIAO ESP32-C3

## 📋 Tabela połączeń wszystkich komponentów

### 🎯 RC522 RFID Reader → ESP32-C3

| Pin RC522 | Pin ESP32-C3 | GPIO | Kolor przewodu | Uwagi |
|-----------|--------------|------|----------------|-------|
| **SDA (SS)** | **D2** | **GPIO 3** | 🟡 Żółty | Chip Select - wybór urządzenia SPI |
| **SCK** | **D8** | **GPIO 7** | 🟠 Pomarańczowy | Zegar SPI |
| **MOSI** | **D10** | **GPIO 9** | 🔵 Niebieski | Master Out Slave In (ESP→RC522) |
| **MISO** | **D9** | **GPIO 8** | 🟢 Zielony | Master In Slave Out (RC522→ESP) |
| **IRQ** | - | - | - | Nie podłączony (opcjonalny) |
| **GND** | **GND** | - | ⚫ Czarny | Masa |
| **RST** | **D1** | **GPIO 2** | 🟣 Fioletowy | Reset czytnika |
| **3.3V** | **3V3** | - | 🔴 Czerwony | Zasilanie 3.3V (NIE 5V!) |

**⚠️ WAŻNE:** RC522 działa tylko na 3.3V! Podłączenie 5V spali moduł!

---

### ⌨️ Keypad MOD-01681 (3x4) → PCF8574 I²C Expander → ESP32-C3

**PCF8574 → ESP32-C3 (I²C):**

| Pin PCF8574 | Pin ESP32-C3 | GPIO | Funkcja |
|-------------|--------------|------|---------|
| **SDA** | **D4** | **GPIO 5** | I²C Data |
| **SCL** | **D5** | **GPIO 6** | I²C Clock |
| **VCC** | **3V3** | - | Zasilanie 3.3V |
| **GND** | **GND** | - | Masa |
| **A0/A1/A2** | **GND** | - | Adres I²C = 0x20 |

**Keypad → PCF8574:**

| Pin PCF8574 | Funkcja keypad |
|-------------|----------------|
| P0 | Row 1 (1, 2, 3) |
| P1 | Row 2 (4, 5, 6) |
| P2 | Row 3 (7, 8, 9) |
| P3 | Row 4 (*, 0, #) |
| P4 | Column 1 (środkowa: 2,5,8,0) |
| P5 | Column 2 (lewa: 1,4,7,*) |
| P6 | Column 3 (prawa: 3,6,9,#) |
| P7 | Unused |

---

### 🕐 DS3231 RTC (Real Time Clock) → ESP32-C3

| Pin DS3231 | Pin ESP32-C3 | GPIO | Funkcja |
|------------|--------------|------|---------|
| **SDA** | **D4** | **GPIO 5** | I²C Data (wspólna z PCF8574) |
| **SCL** | **D5** | **GPIO 6** | I²C Clock (wspólna z PCF8574) |
| **VCC** | **3V3** | - | Zasilanie 3.3V |
| **GND** | **GND** | - | Masa |
| SQW | - | - | Nie używany |
| 32K | - | - | Nie używany |

**💡 Uwaga:** DS3231 i PCF8574 dzielą tę samą magistralę I²C (SDA/SCL)

---

### 🔒 Elektrozamek 12V → MOSFET → ESP32-C3

**Schemat:**
```
ESP32 GPIO4 ──┬──[10kΩ]──> GND
               │
               └──────────> MOSFET Gate (IRF520N)

MOSFET Source ───────────> GND

MOSFET Drain ─────────────┬──> Elektrozamek (-)
                          │
12V Zasilacz (+) ─────────┴──> Elektrozamek (+)
                          │
                          └──[Dioda 1N4007]──> GND (zabezpieczenie)
```

| Połączenie | Pin ESP32-C3 | GPIO | Uwagi |
|------------|--------------|------|-------|
| **MOSFET Gate** | **D3** | **GPIO 4** | Sterowanie zamkiem |
| **MOSFET Source** | **GND** | - | Wspólna masa z ESP32 |
| **MOSFET Drain** | Zamek (-) | - | Przełączanie masy |

---

### 🔊 Buzzer (aktywny 3.3V) → ESP32-C3

| Pin Buzzer | Pin ESP32-C3 | GPIO | Uwagi |
|------------|--------------|------|-------|
| **VCC (+)** | **D0** | **GPIO 1** | GPIO jako źródło zasilania |
| **GND (-)** | **GND** | - | Masa |

**💡 Buzzer aktywny** - wystarczy podać 3.3V aby piszczał

---

### 📳 Czujnik wstrząsów (Waveshare 9536) → ESP32-C3

| Pin Czujnika | Pin ESP32-C3 | GPIO | Uwagi |
|--------------|--------------|------|-------|
| **VCC** | **3V3** | - | Zasilanie 3.3V |
| **GND** | **GND** | - | Masa |
| **OUT** | **D6** | **GPIO21** | Sygnał detekcji (LOW gdy wstrząs); dzielony z TX (UART), ale dostępny jako wejście |

---

## 🗺️ Mapa pinów ESP32-C3 (Seeed XIAO)

```
         USB-C
     ┌─────────────┐
     │             │
     │   ESP32-C3  │
     │   XIAO      │
     │             │
┌────┴─────────────┴────┐
│ D0 (GPIO1)  BUZZER+   │
│ D1 (GPIO2)  RFID_RST  │
│ D2 (GPIO3)  RFID_SS   │
│ D3 (GPIO4)  MOSFET    │
│ D4 (GPIO5)  I2C_SDA   │
│ D5 (GPIO6)  I2C_SCL   │
│ D6 (GPIO21) VIBRATION │
│ D7 (GPIO20) -         │
│ D8 (GPIO7)  RFID_SCK  │
│ D9 (GPIO8)  RFID_MISO │
│ D10(GPIO9) RFID_MOSI │
│ 3V3         POWER     │
│ GND         GROUND    │
└───────────────────────┘
```

---

## ✅ Checklist przed uruchomieniem

- [ ] RC522 podłączony do 3.3V (NIE 5V!)
- [ ] Wszystkie masy (GND) połączone razem
- [ ] I²C (SDA/SCL) wspólne dla PCF8574 i DS3231
- [ ] SPI (MISO=GPIO8, MOSI=GPIO9, SCK=GPIO7) dla RC522
- [ ] MOSFET z pull-down rezystorem 10kΩ
- [ ] Dioda zabezpieczająca 1N4007 na elektrozamku
- [ ] Buzzer podłączony do GPIO1 (3.3V wystarczy)
- [ ] Czujnik wstrząsów do GPIO4

---

## 🔧 Test połączeń

Po wgraniu firmware sprawdź w monitorze szeregowym:

```
=== PineLock Firmware ===
Device ID: domek_1
PCF8574 initialized
RTC initialized
RFID initialized - Version: 0x92
RC522 communication OK
Buzzer initialized
Vibration sensor initialized
Hardware initialization complete
```

Jeśli widzisz:
- `Version: 0x00` lub `0xFF` → błąd połączenia RC522
- `ERROR: PCF8574 not found!` → błąd I²C keypad
- `ERROR: RTC not found!` → błąd I²C RTC

---

## 📸 Zdjęcia okablowania

**Kolory przewodów (sugerowane):**
- 🔴 Czerwony = 3.3V
- ⚫ Czarny = GND
- 🟡 Żółty = RFID SS
- 🟣 Fioletowy = RFID RST
- 🟠 Pomarańczowy = RFID SCK
- 🔵 Niebieski = RFID MOSI
- 🟢 Zielony = RFID MISO
- 🟤 Brązowy = I²C SDA
- ⚪ Biały = I²C SCL
