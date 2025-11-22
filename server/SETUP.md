# Server Setup Guide - Krok po kroku 🏠🔐

## 📋 Czego będziesz potrzebować

### Sprzęt:
- **Raspberry Pi 5** (mały komputer)
- **Karta microSD** (32GB lub więcej) - to będzie "dysk twardy"
- **Zasilacz** (5V USB-C, 5A) - kabel do prądu
- **Kabel sieciowy** (Ethernet) lub WiFi
- **Monitor z HDMI** + **kabel microHDMI→HDMI** (do pierwszej konfiguracji)
- **Klawiatura i mysz USB** (do pierwszej konfiguracji)

### Opcjonalnie (dla ESP32):
- **ESP32** (mikrokontroler do zamka)
- **Przekaźnik** (do sterowania zamkiem)
- **Elektrozaczep** lub zamek elektryczny
- **Kabel micro USB** (do programowania ESP32)

---

## 🎯 KROK 1: Przygotowanie Raspberry Pi

### 1.1. Zainstaluj system operacyjny

1. **Pobierz Raspberry Pi Imager** z [raspberrypi.com/software](https://www.raspberrypi.com/software/)
2. **Włóż kartę microSD** do komputera
3. **Uruchom Imager** i wybierz:
   - System: `Raspberry Pi OS (64-bit)`
   - Urządzenie: `Raspberry Pi 5`
   - Karta: Twoja microSD
4. **Kliknij ikonę zębatki** ⚙️ i ustaw:
   - Nazwa hosta: `pinelock-server`
   - Włącz SSH ✓
   - Użytkownik: `pi` (lub dowolny)
   - Hasło: (wybierz silne hasło)
   - WiFi: (jeśli chcesz używać WiFi zamiast kabla)
5. **Kliknij "ZAPISZ"** i potem **"TAK"** aby sformatować kartę

### 1.2. Pierwszy start

1. **Wyjmij kartę microSD** i włóż do Raspberry Pi
2. **Podłącz**:
   - Monitor przez kabel **microHDMI** (port bliżej USB)
   - Klawiaturę i mysz USB
   - Kabel sieciowy (lub będziesz używać WiFi)
3. **Podłącz zasilanie** - Raspberry Pi uruchomi się automatycznie
4. **Poczekaj ~2 minuty** aż system się załaduje
5. **Zaloguj się** używając użytkownika i hasła z kroku 1.1

---

## 🔧 KROK 2: Podstawowa konfiguracja systemu

### 2.1. Zaktualizuj system

Otwórz terminal (czarna ikona na górze ekranu) i wpisz:

```bash
sudo apt-get update
sudo apt-get upgrade -y
```

**Poczekaj ~10-15 minut** na zakończenie aktualizacji.

### 2.2. Sprawdź połączenie internetowe

```bash
ping -c 3 google.com
```

Powinieneś zobaczyć odpowiedzi. Jeśli nie - sprawdź kabel sieciowy lub WiFi.

### 2.3. Sprawdź adres IP (WAŻNE!)

```bash
hostname -I
```

**Zapisz ten adres!** Będziesz go używać do łączenia się z systemem.
Przykład: `192.168.1.100`

---

## 📦 KROK 3: Instalacja PineLock Server

### 3.1. Pobierz kod projektu

```bash
cd ~
git clone https://github.com/twoj-username/PineLock.git
cd PineLock/server
```

### 3.2. Zainstaluj wymagane pakiety

```bash
sudo apt-get install -y python3-pip python3-venv mosquitto mosquitto-clients
```

### 3.3. Stwórz wirtualne środowisko Python

```bash
python3 -m venv venv
source venv/bin/activate
```

*Zobaczysz `(venv)` przed znakiem zachęty - to dobrze!*

### 3.4. Zainstaluj biblioteki Python

```bash
pip install -r requirements.txt
```

**Poczekaj ~5 minut** na instalację wszystkich pakietów.

---

## 🔐 KROK 4: Konfiguracja MQTT (komunikacja z zamkami)

### 4.1. Uruchom MQTT Broker

```bash
sudo systemctl enable mosquitto
sudo systemctl start mosquitto
```

### 4.2. Ustaw hasło dla MQTT (WAŻNE dla bezpieczeństwa!)

```bash
sudo mosquitto_passwd -c /etc/mosquitto/passwd pinelock
```

**Wpisz hasło** (np. `pinelock123`) i **zapamiętaj je!**

### 4.3. Skonfiguruj Mosquitto

```bash
sudo nano /etc/mosquitto/mosquitto.conf
```

**Dodaj te 2 linijki na końcu pliku:**
```
allow_anonymous false
password_file /etc/mosquitto/passwd
```

**Zapisz**: `Ctrl+O`, `Enter`, `Ctrl+X`

### 4.4. Uruchom ponownie MQTT

```bash
sudo systemctl restart mosquitto
```

### 4.5. Sprawdź czy działa

```bash
sudo systemctl status mosquitto
```

Powinieneś zobaczyć **"active (running)"** w kolorze zielonym.

---

## ⚙️ KROK 5: Konfiguracja serwera PineLock

### 5.1. Stwórz plik konfiguracyjny

```bash
cd ~/PineLock/server
cp .env.example .env
nano .env
```

### 5.2. Wypełnij dane (WAŻNE!)

```env
# MQTT - komunikacja z ESP32
MQTT_BROKER_HOST=localhost
MQTT_BROKER_PORT=1883
MQTT_USERNAME=pinelock
MQTT_PASSWORD=pinelock123          # ← TWOJE HASŁO z kroku 4.2

# Dane logowania do panelu
ADMIN_USERNAME=admin
ADMIN_PASSWORD=admin123            # ← ZMIEŃ NA SWOJE HASŁO!

# Klucz sesji (wygeneruj losowy)
SESSION_SECRET_KEY=twoj_bardzo_tajny_klucz_123456
```

**Zapisz**: `Ctrl+O`, `Enter`, `Ctrl+X`

---

## 🚀 KROK 6: Uruchom serwer!

### 6.1. Szybki test (ręcznie)

```bash
cd ~/PineLock/server
source venv/bin/activate
./start.sh
```

Powinieneś zobaczyć:
```
╔═══════════════════════════════════════════════╗
║         🌲 PineLock Server Started 🔐         ║
╚═══════════════════════════════════════════════╝

🌐 Adresy dostępu:
   • Localhost:     http://127.0.0.1:8000
   • Sieć lokalna:  http://192.168.1.100:8000  ← TWÓJ ADRES

📱 Panel webowy:     http://192.168.1.100:8000/ui/login
👤 Domyślne dane:    admin / admin123
```

### 6.2. Testuj w przeglądarce!

Na **dowolnym urządzeniu w tej samej sieci** (telefon, laptop):
1. Otwórz przeglądarkę
2. Wpisz: `http://TWOJ_ADRES_IP:8000/ui/login`
3. Zaloguj się: `admin` / `admin123`

**DZIAŁA? SUPER! 🎉**

Zatrzymaj serwer: `Ctrl+C`

---

## ⚡ KROK 7: Automatyczne uruchamianie (opcjonalne, ale zalecane)

### 7.1. Stwórz usługę systemową

```bash
sudo nano /etc/systemd/system/pinelock.service
```

**Wklej ten kod** (zmień `pi` na swoją nazwę użytkownika jeśli inna):

```ini
[Unit]
Description=PineLock Server - System zarządzania domkami
After=network.target mosquitto.service

[Service]
Type=simple
User=pi
WorkingDirectory=/home/pi/PineLock/server
Environment="PATH=/home/pi/PineLock/server/venv/bin"
ExecStart=/home/pi/PineLock/server/venv/bin/uvicorn app.main:app --host 0.0.0.0 --port 8000
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

**Zapisz**: `Ctrl+O`, `Enter`, `Ctrl+X`

### 7.2. Włącz auto-start

```bash
sudo systemctl daemon-reload
sudo systemctl enable pinelock
sudo systemctl start pinelock
```

### 7.3. Sprawdź status

```bash
sudo systemctl status pinelock
```

Powinieneś zobaczyć **"active (running)"** - serwer działa!

### 7.4. Przydatne komendy

```bash
# Zatrzymaj serwer
sudo systemctl stop pinelock

# Uruchom serwer
sudo systemctl start pinelock

# Restart serwera
sudo systemctl restart pinelock

# Zobacz logi (ostatnie błędy/komunikaty)
sudo journalctl -u pinelock -f
```

---

## 🔌 KROK 8: Podłącz ESP32 (zamek)

### 8.1. Schemat połączenia ESP32

```
ESP32                    Przekaźnik               Zamek/Elektrozaczep
─────                    ──────────               ───────────────────
                         
GPIO 2 ────────────────> IN                       
3.3V ──────────────────> VCC                      
GND ───────────────────> GND                      
                         
                         NO (Normally Open) ────> (+) Zasilanie zamka
                         COM ────────────────────> (+) Zasilanie 12V
                         
                         GND ─────────────────────> (-) Zamek
                                                   (-) Zasilanie 12V
```

### 8.2. WiFi i MQTT w ESP32

W pliku `config.h` na ESP32 ustaw:

```cpp
// WiFi - TA SAMA SIEĆ co Raspberry Pi!
#define WIFI_SSID "TwojaNazwaWiFi"
#define WIFI_PASSWORD "TwojeHasloWiFi"

// MQTT - adres Raspberry Pi
#define MQTT_BROKER "192.168.1.100"  // ← TWÓJ ADRES IP z kroku 2.3
#define MQTT_PORT 1883
#define MQTT_USERNAME "pinelock"
#define MQTT_PASSWORD "pinelock123"   // ← hasło z kroku 4.2

// ID zamka (unikalny dla każdego ESP32)
#define LOCK_ID "domek_1"
```

### 8.3. Wgraj kod na ESP32

```bash
cd ~/PineLock/firmware/lock_node
pio run --target upload
```

### 8.4. Sprawdź komunikację

W terminalu Raspberry Pi:

```bash
mosquitto_sub -h localhost -u pinelock -P pinelock123 -t "pinelock/#" -v
```

Uruchom ESP32 - powinieneś zobaczyć komunikaty:
```
pinelock/domek_1/status online
pinelock/domek_1/state locked
```

**DZIAŁA? JESTEŚ GOTOWY! 🎉**

---

## 📊 Diagram połączeń całego systemu

```
┌─────────────────────────────────────────────────────────────────┐
│                        TWOJA SIEĆ WiFi                          │
│                         192.168.1.x                             │
└─────────────────────────────────────────────────────────────────┘
         │                       │                      │
         │                       │                      │
    ┌────▼─────┐          ┌─────▼──────┐         ┌────▼─────┐
    │ Telefon  │          │ Raspberry  │         │  ESP32   │
    │          │          │   Pi 5     │         │ (Domek 1)│
    │ Chrome   │◄────────►│            │◄───────►│          │
    │          │   HTTP   │ - Python   │  MQTT   │ - WiFi   │
    │          │          │ - MQTT     │         │ - GPIO 2 │
    └──────────┘          │ - SQLite   │         └────┬─────┘
                          └────────────┘              │
                                                      │
                                                 ┌────▼─────┐
                                                 │Przekaźnik│
                                                 └────┬─────┘
                                                      │
                                                 ┌────▼─────┐
                                                 │  ZAMEK   │
                                                 │    🔐     │
                                                 └──────────┘
```

### Jak to wszystko działa?

1. **Otwierasz panel w przeglądarce** na telefonie/komputerze
2. **Klikasz "Otwórz zamek"** dla Domek 1
3. **Raspberry Pi** wysyła komendę przez **MQTT**
4. **ESP32** odbiera komendę i włącza **GPIO 2**
5. **Przekaźnik** zamyka obwód i **zamek się otwiera**
6. **ESP32** potwierdza wykonanie i wysyła status z powrotem
7. **Panel** pokazuje "Otwarte" ✅

---

## ✅ Checklist - Co powinieneś mieć

- [ ] Raspberry Pi działa i ma internet
- [ ] Znasz adres IP Raspberry Pi (np. `192.168.1.100`)
- [ ] MQTT broker działa (`sudo systemctl status mosquitto`)
- [ ] Serwer PineLock działa (`sudo systemctl status pinelock`)
- [ ] Możesz otworzyć panel w przeglądarce (`http://IP:8000/ui/login`)
- [ ] Możesz się zalogować (admin / admin123)
- [ ] ESP32 podłączony do WiFi i MQTT
- [ ] Przekaźnik podłączony do ESP32 (GPIO 2)
- [ ] Zamek podłączony do przekaźnika

---

## 🆘 Co robić gdy coś nie działa?

### Problem: Nie mogę otworzyć panelu w przeglądarce

**Rozwiązanie:**
```bash
# Sprawdź czy serwer działa
sudo systemctl status pinelock

# Zobacz logi
sudo journalctl -u pinelock -n 50

# Sprawdź adres IP
hostname -I

# Uruchom ręcznie żeby zobaczyć błędy
cd ~/PineLock/server
source venv/bin/activate
./start.sh
```

### Problem: ESP32 nie łączy się z MQTT

**Rozwiązanie:**
1. Sprawdź czy ESP32 ma ten sam WiFi co Raspberry Pi
2. Sprawdź adres IP w `config.h` - musi być taki jak Raspberry Pi
3. Sprawdź hasło MQTT - musi być takie samo jak w `.env`
4. Sprawdź logi ESP32: `pio device monitor`

### Problem: Zamek się nie otwiera

**Rozwiązanie:**
1. Sprawdź połączenie przekaźnika (GPIO 2 z ESP32)
2. Sprawdź zasilanie zamka (12V)
3. Testuj przekaźnik ręcznie - podepnij 3.3V do IN
4. Sprawdź logi: `mosquitto_sub -h localhost -u pinelock -P pinelock123 -t "pinelock/#" -v`

### Problem: "Permission denied" przy instalacji

**Rozwiązanie:**
```bash
# Dodaj sudo przed komendą
sudo apt-get install ...
```

### Problem: Zapomniałem hasła do panelu

**Rozwiązanie:**
```bash
cd ~/PineLock/server
nano .env
# Zmień ADMIN_PASSWORD na nowe hasło
sudo systemctl restart pinelock
```

---

## 📞 Potrzebujesz pomocy?

1. **Sprawdź logi**:
   ```bash
   # Logi serwera
   sudo journalctl -u pinelock -n 100
   
   # Logi MQTT
   sudo journalctl -u mosquitto -n 100
   ```

2. **Testuj MQTT**:
   ```bash
   # Nasłuchuj wszystkich wiadomości
   mosquitto_sub -h localhost -u pinelock -P pinelock123 -t "#" -v
   
   # Wyślij test
   mosquitto_pub -h localhost -u pinelock -P pinelock123 -t "test" -m "hello"
   ```

3. **Sprawdź połączenia**:
   ```bash
   # Pokaż adres IP
   hostname -I
   
   # Sprawdź czy port 8000 jest otwarty
   sudo netstat -tulpn | grep 8000
   
   # Sprawdź czy MQTT działa
   sudo netstat -tulpn | grep 1883
   ```

---

## 🎓 Glossary (Słowniczek)

- **Raspberry Pi** - Mały komputer wielkości karty kredytowej
- **ESP32** - Mikrokontroler z WiFi (mózg zamka)
- **MQTT** - Protokół komunikacji między urządzeniami (jak WhatsApp dla IoT)
- **Broker** - Serwer MQTT (pośrednik przesyłający wiadomości)
- **GPIO** - Piny elektroniczne na ESP32 (wyjścia/wejścia)
- **Przekaźnik** - Przełącznik elektroniczny (włącza/wyłącza zamek)
- **Virtual Environment (venv)** - Odizolowane środowisko dla bibliotek Python
- **systemd** - System zarządzający usługami w Linux

---

## 🚀 Gratulacje!

Jeśli dotarłeś tutaj - **masz działający system PineLock**! 

Możesz teraz:
- ✅ Zarządzać zamkami przez przeglądarkę
- ✅ Dodawać kody PIN
- ✅ Przeglądać historię dostępu
- ✅ Monitorować status domków

**Następne kroki:**
1. Zmień domyślne hasła (`.env`)
2. Dodaj więcej zamków/ESP32
3. Skonfiguruj kopie zapasowe bazy danych
4. Ustaw firewall dla bezpieczeństwa

---

## 📚 Przydatne linki

- [Dokumentacja Raspberry Pi](https://www.raspberrypi.com/documentation/)
- [MQTT Tutorial](https://mqtt.org/getting-started/)
- [PlatformIO - ESP32](https://docs.platformio.org/en/latest/boards/espressif32/esp32dev.html)
- [FastAPI Docs](https://fastapi.tiangolo.com/)

---

**Autor**: PineLock Team  
**Wersja**: 1.0  
**Data**: 2025
