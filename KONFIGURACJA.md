# PineLock - Kompletna Dokumentacja Konfiguracji

## 📋 Informacje o urządzeniu

- **Device ID:** `domek_1`
- **Płytka:** Seeed XIAO ESP32C3
- **IP adres:** 192.168.1.31
- **Serwer IP:** 192.168.1.13

## 🔐 Konfiguracja WiFi i MQTT

### WiFi
- **SSID:** `Orange_Swiatlowod_DDC0`
- **Hasło:** `M3WGS27SRMY6`
- Dane zapisane w: `firmware/lock_node/include/config.h`

### MQTT Broker
- **Host:** `192.168.1.13` (localhost na serwerze)
- **Port:** `1883`
- **Autentykacja:** Brak (anonymous allowed)
- **Keepalive:** 120 sekund
- **Topic prefix:** `pinelock`

### Konfiguracja Mosquitto
Plik: `/etc/mosquitto/conf.d/default.conf`
```
listener 1883
allow_anonymous true
```

## 🔌 Mapowanie GPIO (Seeed XIAO ESP32C3)

| Funkcja | GPIO | Pin | Opis |
|---------|------|-----|------|
| **I2C SDA** | 6 | D4 | Klawiatura (PCF8574) + RTC (DS3231) |
| **I2C SCL** | 7 | D5 | Klawiatura (PCF8574) + RTC (DS3231) |
| **Zamek** | 10 | D10 | Sterowanie MOSFET (HIGH=otwarty) |
| **RFID SS** | 3 | D3 | MFRC522 Chip Select |
| **RFID RST** | 2 | D2 | MFRC522 Reset |
| **RFID MISO** | 4 | - | SPI |
| **RFID MOSI** | 5 | - | SPI |
| **RFID SCK** | 8 | D8 | SPI Clock |

### Adresy I2C
- **PCF8574 (Klawiatura):** `0x20` (domyślnie, może być `0x27`)
- **DS3231 (RTC):** `0x68`

## 🚀 Wgrywanie Firmware

### Normalny tryb
```bash
./upload_firmware.sh
```

### Tryb bootloader (jeśli upload failed)
1. Trzymaj przycisk **BOOT** (lewy)
2. Naciśnij krótko **RESET** (prawy)
3. Puść **BOOT**
4. Uruchom `./upload_firmware.sh`

### Monitorowanie urządzenia
```bash
./monitor_device.sh
```

Po wgraniu firmware naciśnij **RESET** (prawy przycisk), aby uruchomić program.

## 🖥️ Uruchamianie Serwera

```bash
cd server
./start.sh
```

Serwer automatycznie:
- Instaluje i uruchamia Mosquitto (jeśli nie działa)
- Aktywuje środowisko Python
- Uruchamia FastAPI na porcie 8000

**Panel webowy:** http://localhost:8000 lub http://192.168.1.13:8000  
**Login:** `admin` / `wkswks12`

## 🔧 Rozwiązywanie problemów

### Klawiatura nie działa (`ERROR: PCF8574 not found!`)

1. Sprawdź połączenia:
   - **VCC** → 3.3V lub 5V
   - **GND** → GND
   - **SDA** → Pin D4 (GPIO 6)
   - **SCL** → Pin D5 (GPIO 7)

2. Sprawdź adres I2C (może być 0x27 zamiast 0x20)
3. Upewnij się, że kable SDA i SCL nie są zamienione

### Urządzenie się ciągle resetuje

**Objaw:** W logach widzisz `Task watchdog got triggered`

**Rozwiązanie:** Upewnij się, że firmware ma ustawiony MQTT keepalive na 120s:
```cpp
mqttClient.setKeepAlive(120);
```

### MQTT timeout / Connection reset

**Objaw:** `select returned due to timeout` lub `Connection reset by peer`

**Przyczyny:**
1. Mosquitto nie nasłuchuje na właściwym porcie
2. Firewall blokuje port 1883
3. Urządzenie ma zły IP brokera w `config.h`

**Sprawdzenie:**
```bash
ss -tlnp | grep 1883  # Powinno pokazać 0.0.0.0:1883
```

### Port USB zajęty (`Resource temporarily unavailable`)

Zamknij `monitor_device.sh` (Ctrl+C) przed wgrywaniem firmware.

### Monitor pokazuje `wait usb download`

Płytka jest w trybie bootloader. Naciśnij **RESET** (prawy przycisk), aby uruchomić program.

## 🎮 Sterowanie zamkiem

### Przez MQTT (zdalnie)
```bash
# Otwarcie
./server/venv/bin/python -c "import paho.mqtt.publish as publish; import json; publish.single('pinelock/domek_1/command', json.dumps({'action': 'unlock'}), hostname='127.0.0.1', port=1883, qos=1)"

# Zamknięcie
./server/venv/bin/python -c "import paho.mqtt.publish as publish; import json; publish.single('pinelock/domek_1/command', json.dumps({'action': 'lock'}), hostname='127.0.0.1', port=1883, qos=1)"
```

### Przez panel webowy
1. Wejdź na http://localhost:8000
2. Zaloguj się (`admin` / `wkswks12`)
3. Dodaj zamek: **Device ID:** `domek_1`
4. Kliknij **Unlock** / **Lock**

### Przez klawiaturę (gdy podłączona)
1. Wpisz PIN (np. `1234`)
2. Naciśnij `#` aby zatwierdzić
3. Naciśnij `*` aby skasować wpisany kod

**Uwaga:** Musisz najpierw dodać kod PIN przez panel webowy lub MQTT.

### Przez kartę RFID (gdy podłączona)
Przyłóż kartę do czytnika. Karta musi być wcześniej dodana do systemu.

## 📝 Logi i diagnostyka

### Sprawdzenie statusu MQTT
```bash
sudo tail -f /var/log/mosquitto/mosquitto.log | grep domek_1
```

### Sprawdzenie wiadomości MQTT (test)
```bash
mosquitto_sub -h 127.0.0.1 -t "pinelock/#" -v
```

### Test publikowania wiadomości
```bash
mosquitto_pub -h 127.0.0.1 -t "pinelock/domek_1/command" -m '{"action":"unlock"}'
```

## 📂 Ważne pliki

| Plik | Opis |
|------|------|
| `firmware/lock_node/include/config.h` | Konfiguracja WiFi, MQTT, GPIO |
| `firmware/lock_node/wifi_credentials.txt` | Backup danych WiFi |
| `server/.env` | Konfiguracja serwera (NIE zawiera WiFi!) |
| `/etc/mosquitto/conf.d/default.conf` | Konfiguracja Mosquitto |
| `GPIO_PINOUT.md` | Mapowanie pinów GPIO |

## 🔄 Typowy przepływ pracy

1. **Uruchom serwer** (jednorazowo):
   ```bash
   cd /home/kmush/Desktop/Work/PineLock/server
   ./start.sh
   ```

2. **Wgraj firmware** (po zmianach w kodzie):
   ```bash
   cd /home/kmush/Desktop/Work/PineLock
   ./upload_firmware.sh
   ```

3. **Monitoruj urządzenie** (debugowanie):
   ```bash
   ./monitor_device.sh
   ```

4. **Otwórz zamek zdalnie**:
   - Panel web: http://localhost:8000
   - Lub komenda MQTT (patrz wyżej)

## 🎯 Auto-lock

Zamek **automatycznie zamyka się** po **5 sekundach** od otwarcia.

Zmiana czasu w `config.h`:
```cpp
#define LOCK_DURATION 5000  // 5 sekund (w milisekundach)
```

## 🛡️ Bezpieczeństwo

- **MQTT:** Obecnie bez hasła (anonymous). Dla produkcji ustaw MQTT_USERNAME i MQTT_PASSWORD.
- **Panel web:** Zmień domyślne hasło w `server/.env`:
  ```
  ADMIN_PASSWORD=twoje_nowe_haslo
  ```
- **WiFi:** Dane w `config.h` są w plain text - zabezpiecz dostęp do firmware!

## 📞 Kontakt z urządzeniem

**MQTT Topics:**
- Komendy: `pinelock/domek_1/command`
- Status: `pinelock/domek_1/status`
- Heartbeat: `pinelock/domek_1/heartbeat`
- Zdarzenia dostępu: `pinelock/domek_1/access`
