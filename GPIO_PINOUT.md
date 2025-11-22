# Seeed XIAO ESP32C3 - Mapowanie GPIO

## Mapowanie pinów D → GPIO

| Pin na płytce | GPIO | Użycie w PineLock |
|---------------|------|-------------------|
| D0 | GPIO 0 | - |
| D1 | GPIO 1 | - |
| D2 | GPIO 2 | RFID RST |
| D3 | GPIO 3 | RFID SS |
| D4 | GPIO 6 | **I2C SDA** (Klawiatura, RTC) |
| D5 | GPIO 7 | **I2C SCL** (Klawiatura, RTC) |
| D6 | GPIO 21 | - |
| D7 | GPIO 20 | - |
| D8 | GPIO 8 | RFID SCK |
| D9 | GPIO 9 | - |
| D10 | GPIO 10 | **Zamek (MOSFET)** |

## Piny SPI (RFID - MFRC522)

| Funkcja | GPIO | Pin |
|---------|------|-----|
| MISO | GPIO 4 | - |
| MOSI | GPIO 5 | - |
| SCK | GPIO 8 | D8 |
| SS | GPIO 3 | D3 |
| RST | GPIO 2 | D2 |

## Piny I2C (Klawiatura 4x4 + RTC DS3231)

| Funkcja | GPIO | Pin |
|---------|------|-----|
| SDA | GPIO 6 | D4 |
| SCL | GPIO 7 | D5 |

**Adres I2C:**
- Klawiatura (PCF8574): `0x20` (domyślnie)
- RTC (DS3231): `0x68`

## Sterowanie zamkiem

| Funkcja | GPIO | Pin | Stan |
|---------|------|-----|------|
| MOSFET Gate | GPIO 10 | D10 | HIGH = Otwarty, LOW = Zamknięty |

---

## Uwagi

1. **Klawiatura:** Jeśli firmware pokazuje `ERROR: PCF8574 not found!`, sprawdź połączenie SDA/SCL lub adres I2C (może być 0x27 zamiast 0x20).
2. **RFID:** Wymaga zasilania 3.3V, GND i 5 przewodów SPI.
3. **Zamek:** Używa MOSFET do sterowania elektromagnesem (zalecane 12V).
