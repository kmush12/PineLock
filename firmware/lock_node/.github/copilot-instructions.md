# PineLock Copilot Instructions

## Architecture
- `src/main.cpp` is the single loop controller: setup configures WDT (30s), hardware, WiFi, MQTT; `loop()` must stay non-blocking because it feeds the watchdog and multiplexes keypad, RFID, vibration, buzzer and heartbeat.
- `include/access_control.h` + `src/access_control.cpp` hold in-memory PIN/RFID lists (max 50 each) backed by `Preferences` namespace `access_ctrl`; arrays are authoritative, so always modify them via the class API.
- Hardware abstractions stay lightweight: PCF8574 powers the keypad matrix, MFRC522 is polled over SPI, DS3231 supplies wall clock data, and GPIO pins for MOSFET, buzzer, and vibration sensor are driven directly.

## Configuration & Secrets
- Never edit production credentials in code samples—real deployments must copy `include/config.h.example` to `include/config.h` and fill WiFi, MQTT, and `DEVICE_ID`; repo copy currently contains lab creds only.
- Custom hardware pin maps live in `config.h`; keep them consistent with `SETUP.md` wiring diagrams before changing keypad, RFID, or sensor wiring.
- `wifi_credentials.txt` is a human note; firmware reads only `config.h`, so keep those two sources aligned manually.

## Build, Flash, Monitor
- PlatformIO environment is `[env:esp32-c3]` (`platformio.ini`); use `pio run` for compile, `pio run --target upload` (default port `/dev/ttyACM0`) for flashing, and `pio device monitor -b 115200 --filter esp32_exception_decoder` for logs.
- Release flags already enable `-O2`, `-Wall`, `-Wextra`, and `CORE_DEBUG_LEVEL=3`; keep new C++ files under `src/` so PlatformIO picks them up automatically.

## Runtime Data Flow
- Keypad scanning (`handleKeypad` ➜ `readKeypad` ➜ `processPINEntry`) toggles PCF8574 columns one at a time; submit PINs with `#`, reset with `*`, digits capped at 10 chars. Respect `KEYPAD_KEYS` ordering when adding new symbols.
- Access decisions happen locally: valid PINs unlock immediately via `controlLock(false)` and publish an `access` event; failed attempts buzz for `BUZZER_WRONG_PIN_DURATION` and emit the same event with `success=false`.
- RFID presence is polled every `RFID_CHECK_INTERVAL_MS`; only changes trigger `sendKeyStatusUpdate`, which reuses the `.../status` topic—avoid separate status publishers to keep telemetry coherent.
- Vibration sensor on `VIBRATION_SENSOR_PIN` fires `handleVibration`, triggering buzzer + MQTT `.../alert`. Keep that ISR-free design (polling with debounce) unless you fully audit watchdog timing.

## MQTT Contract
- Topic prefix comes from `MQTT_TOPIC_PREFIX` (default `pinelock`); every publish/subscribe is `prefix/DEVICE_ID/<suffix>`.
- Device publishes `heartbeat`, `status` (also carries key presence), `access`, and `alert`; it subscribes to `command` (`lock`/`unlock`) and `sync` (currently TODO—see `mqttCallback`). Document new payloads in `README.md` to stay aligned with backend devs.

## Persistence & Time Rules
- `AccessControl::saveToEEPROM` only stores the code string + `isActive`. Time windows (`validFrom/validUntil`) are not persisted, so time-limited entries revert to "always allowed" after reboot; if you extend persistence, mirror both structs and honor `hasTimeLimit` when loading.
- RTC failures default to "allow" semantics (`isWithinValidTime` returns true without an RTC), so guard against clock loss manually if stricter security is required.

## Conventions & Pitfalls
- Keep new logic non-blocking and feed the watchdog (`esp_task_wdt_reset`) during any retry loops (see WiFi/MQTT connect patterns). Long delays require chunked loops rather than `delay()` calls.
- Prefer `StaticJsonDocument` sized to payloads (see existing 128/256 byte documents) to avoid heap churn.
- When extending MQTT commands (e.g., add PIN via backend), validate payload fields before acting, reuse `sendAccessEvent` for auditing, and call `saveToEEPROM` after mutating credentials.
- Hardware init guards (`pcf8574Found`, `rtcFound`) exist for degraded modes—respect them before reading hardware or timestamps.
