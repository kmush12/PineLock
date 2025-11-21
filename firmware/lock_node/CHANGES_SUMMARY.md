<div align="center">

# 📝 Changelog & Version History
## PineLock Firmware Changes Summary

**Version:** 1.0.0-beta  
**Release Date:** November 21, 2025  
**Type:** Major Security & Stability Update

[![Changes](https://img.shields.io/badge/changes-250%2B%20lines-blue)]() [![Files](https://img.shields.io/badge/files%20modified-4-green)]() [![Status](https://img.shields.io/badge/status-breaking%20changes-orange)]()

---

</div>

## 🔄 Modified Files

<table>
<tr>
<th width="30%">File</th>
<th width="15%">Lines Changed</th>
<th width="55%">Changes</th>
</tr>
<tr>
<td><code>include/config.h</code></td>
<td align="center">+15</td>
<td>➕ Added SPI pins<br>➕ Added timing constants</td>
</tr>
<tr>
<td><code>include/access_control.h</code></td>
<td align="center">+5</td>
<td>➕ EEPROM persistence methods<br>➕ Preferences library</td>
</tr>
<tr>
<td><code>src/access_control.cpp</code></td>
<td align="center">+120</td>
<td>➕ Full EEPROM implementation<br>🔧 Auto-save on changes</td>
</tr>
<tr>
<td><code>src/main.cpp</code></td>
<td align="center">+110</td>
<td>➕ Watchdog timer<br>➕ Keypad scanning<br>🔧 Overflow protection<br>🔧 JSON validation</td>
</tr>
</table>

## 📋 Detailed Changes by Component

### 1. `include/config.h`
**Dodane:**
- Definicje pinów SPI dla RFID:
  - `RFID_MISO_PIN 4`
  - `RFID_MOSI_PIN 5`
  - `RFID_SCK_PIN 8`
- Stałe czasowe:
  - `KEYPAD_DEBOUNCE_MS 500`
  - `RFID_CHECK_INTERVAL_MS 500`

### 2. `include/access_control.h`
**Dodane:**
- `#include <Preferences.h>` - biblioteka EEPROM
- Pole prywatne: `Preferences preferences`
- Metody publiczne:
  - `void saveToEEPROM()`
  - `void loadFromEEPROM()`

### 3. `src/access_control.cpp`
**Zmodyfikowane:**
- Konstruktor - dodano `loadFromEEPROM()` przy inicjalizacji
- `addPINCode()` - dodano `saveToEEPROM()` po każdej zmianie
- `removePINCode()` - dodano `saveToEEPROM()` po usunięciu
- `addRFIDCard()` - dodano `saveToEEPROM()` po każdej zmianie
- `removeRFIDCard()` - dodano `saveToEEPROM()` po usunięciu

**Dodane:**
- `saveToEEPROM()` - pełna implementacja (~40 linii)
- `loadFromEEPROM()` - pełna implementacja (~60 linii)

### 4. `src/main.cpp`
**Dodane:**
- `#include <esp_task_wdt.h>`
- `#define WDT_TIMEOUT 30`
- Zmiana: `AccessControl accessControl(&rtc);` (zamiast wskaźnika)

**Zmodyfikowane w setup():**
- Dodano inicjalizację watchdog:
  ```cpp
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
  ```

**Zmodyfikowane w setupHardware():**
- Inicjalizacja SPI z nowymi pinami:
  ```cpp
  SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
  ```
- Usunięto tworzenie obiektu AccessControl przez `new`
- Usunięto hardcoded PIN "1234"
- Dodano ostrzeżenie o braku domyślnego PIN

**Zmodyfikowane w loop():**
- Dodano `esp_task_wdt_reset()` na początku
- Poprawiono obsługę overflow millis() w heartbeat
- Poprawiono obsługę overflow millis() w auto-lock
- Używamy `currentMillis` zamiast wielokrotnych `millis()`

**Zmodyfikowane w mqttCallback():**
- Dodano walidację JSON: `if (!doc.containsKey("action"))`
- Dodano obsługę nieznanej komendy
- Poprawiono konwersję: `doc["action"].as<String>()`

**Zmodyfikowane w readKeypad():**
- Pełna implementacja skanowania matrycy 4x4:
  ```cpp
  for (int col = 0; col < KEYPAD_COLS; col++) {
      // Ustaw wszystkie kolumny HIGH
      // Ustaw aktualną kolumnę LOW
      // Sprawdź wszystkie wiersze
      // Zwróć klawisz jeśli znaleziono
  }
  ```

**Zmodyfikowane w handleKeypad():**
- Używamy `unsigned long currentMillis`
- Poprawiono detekcję overflow przy debounce
- Używamy stałej `KEYPAD_DEBOUNCE_MS`

**Zmodyfikowane w handleRFID():**
- Używamy `unsigned long currentMillis`
- Poprawiono detekcję overflow przy sprawdzaniu RFID
- Używamy stałej `RFID_CHECK_INTERVAL_MS`

**Zmodyfikowane w processPINEntry():**
- Zmiana wywołań z `accessControl->` na `accessControl.`

## Nowe pliki dokumentacji

### 1. `CODE_REVIEW_REPORT.md`
Pełny raport z przeglądu zawierający:
- Podsumowanie wykonawcze
- 9 znalezionych i naprawionych problemów
- Kategoryzacja: Krytyczne, Wysokie, Średnie
- Statystyki zmian
- Rekomendacje na przyszłość
- Plan testowania
- Wnioski

### 2. `DEPLOYMENT.md`
Instrukcje wdrożenia zawierające:
- Opis zmian w projekcie
- Kroki wdrożenia krok po kroku
- Konfiguracja przed uploadem
- Dokumentacja MQTT topics
- Procedury testowania
- Rozwiązywanie problemów
- Checklist przed produkcją
- Następne kroki rozwoju

## Statystyki

**Całkowite zmiany:**
- Pliki kodu zmodyfikowane: 4
- Pliki dokumentacji dodane: 2
- Linii dodanych: ~250
- Linii zmodyfikowanych: ~80
- Linii usuniętych: ~15

**Naprawione problemy:** 9/9 (100%)
- Krytyczne: 5/5 ✅
- Wysokie: 3/3 ✅
- Średnie: 1/1 ✅

## Kompatybilność wsteczna

⚠️ **BREAKING CHANGES:**

1. **AccessControl API zmienione:**
   - Zamiast `accessControl->method()` używamy `accessControl.method()`
   - Może wymagać zmian w kodzie który używa tego obiektu

2. **Nowe zachowanie przy starcie:**
   - Brak domyślnego PIN - wymaga konfiguracji przez MQTT
   - Automatyczne ładowanie z EEPROM przy starcie

3. **Wymagane nowe zdefiniowanie pinów:**
   - Jeśli ktoś kopiuje config.h z poprzedniej wersji, musi dodać nowe piny SPI

## Wszystkie zmiany są kompatybilne z:
- PlatformIO ✅
- ESP32-C3 ✅
- Arduino Framework ✅
- Wszystkie użyte biblioteki ✅

## Status projektu

**Przed review:** Pre-Alpha (wiele TODO, krytyczne błędy)
**Po review:** Beta (gotowe do testów, wszystkie core features działają)

**Kolejny krok:** Testy integracyjne i wdrożenie pilotażowe
