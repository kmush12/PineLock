# 🔧 Instrukcja montażu ESP32 - Krok po kroku

## 📦 Co będziesz potrzebować

### 🛠️ Części elektroniczne:

| Co to jest? | Model/Typ | Ile sztuk | Do czego? |
|-------------|-----------|-----------|-----------|
| 🔲 **Mózg systemu** | ESP32-C3 (Seeed XIAO) | 1 | Steruje wszystkim |
| 🔌 **Kabel USB** | USB-C (z danymi!) | 1 | Do programowania |
| 🎹 **Klawiatura** | MOD-01681 (3x4, 12 klawiszy) | 1 | Wpisywanie kodu PIN |
| 🔧 **Rozszerzenie** | PCF8574 (moduł I2C) | 1 | Podłączenie klawiatury |
| 📡 **Czytnik kart** | RC522 (czytnik RFID) | 1 | Wykrywanie kluczy RFID |
| ⏰ **Zegar** | DS3231 (moduł RTC) | 1 | Pamięta czas |
| 🔋 **Bateria** | CR2032 (bateria płaska) | 1 | Zasilanie zegara |
| ⚡ **Przełącznik** | Moduł MOSFET | 1 | Włącza zamek |
| 🔒 **Zamek** | Elektrozaczep 12V | 1 | Fizyczny zamek |
| 🔌 **Zasilacz** | 12V DC | 1 | Prąd dla zamka |
| 🔊 **Buzzer** | Buzzer aktywny 3.3V | 1 | Dźwięk alarmu |
| 📳 **Czujnik wstrząsów** | Waveshare 9536 | 1 | Wykrywa wibracje |
| 🔗 **Kabelki** | Przewody Dupont | Zestaw | Połączenia |

### 💻 Oprogramowanie:

- **PlatformIO** - program do wgrywania kodu (instalacja poniżej)
- **Python 3.7+** - potrzebny dla PlatformIO
- **MQTT Broker** - na Raspberry Pi (z poprzedniego SETUP.md)

---

## 🎯 KROK 1: Zrozum jak to działa

### Prosty schemat:

```
                    📱 Telefon
                       │
                       │ (WiFi)
                       ▼
    ╔══════════════════════════════╗
    ║     Raspberry Pi             ║
    ║  (serwer + MQTT broker)      ║
    ╚══════════════════════════════╝
                       │
                       │ (WiFi + MQTT)
                       ▼
    ╔══════════════════════════════╗
    ║        ESP32-C3              ║
    ║   (mózg zamka)               ║
    ╚══════════════════════════════╝
           │       │       │       │       │
           ▼       ▼       ▼       ▼       ▼
      Klawiatura  RFID   Zamek  Buzzer  Wstrząsy
        (PIN)   (karta)  (🔒)    (🔊)     (📳)
```

### Co się dzieje gdy wpiszesz kod PIN:

1. **Wciskasz** przyciski na klawiaturze → `1`, `2`, `3`, `4`, `#`
2. **ESP32** czyta klawiaturę przez PCF8574
3. **ESP32** wysyła kod do Raspberry Pi przez WiFi/MQTT
4. **Raspberry Pi** sprawdza czy kod jest dobry w bazie danych
5. **Raspberry Pi** odsyła "OK, otwórz!" przez MQTT
6. **ESP32** włącza MOSFET
7. **MOSFET** włącza zamek na 12V
8. **ZAMEK** się otwiera! 🎉

### Co się dzieje gdy wpiszesz ZŁY kod PIN:

1. **Wciskasz** zły kod → np. `9`, `9`, `9`, `9`, `#`
2. **ESP32** sprawdza kod w pamięci
3. **Kod niepoprawny!** ❌
4. **Buzzer piszczy** przez 1 sekundę 🔊
5. ESP32 wysyła informację o nieudanej próbie do serwera

### Co się dzieje gdy ktoś szarpie za zamek:

1. **Czujnik wstrząsów** wykrywa wibracje 📳
2. **ESP32** dostaje sygnał LOW na GPIO 21 (D6)
3. **Buzzer piszczy** przez 1 sekundę 🔊
4. ESP32 wysyła **ALERT** przez MQTT do serwera
5. Możesz dostać powiadomienie na telefon! 📱

---

## 🔌 KROK 2: Podłącz wszystko (WAŻNE!)

### 2.1. Przygotuj miejsce pracy

1. **Rozłóż** wszystkie części na stole
2. **Sprawdź** czy masz wszystko z listy wyżej
3. **Przygotuj** wkrętaki, taśmę izolacyjną, multimetr (opcjonalnie)

### 2.2. Podłącz magistralę I2C (wspólne połączenie)

**To jest jak "szyna" - podłączasz do niej kilka urządzeń:**

#### Co to jest I2C?
To sposób podłączania wielu urządzeń tylko **2 przewodami**! Jak wspólny autobus - wszystkie urządzenia jadą tymi samymi kablami.

#### Podłącz do ESP32:

| Przewód ESP32 | Kolor (sugerowany) | Podłącz do |
|---------------|-------------------|------------|
| **GPIO 6** (SDA) | 🔵 Niebieski | SDA na PCF8574 **I** SDA na DS3231 |
| **GPIO 7** (SCL) | 🟡 Żółty | SCL na PCF8574 **I** SCL na DS3231 |
| **3.3V** | 🔴 Czerwony | VCC na PCF8574 **I** VCC na DS3231 |
| **GND** | ⚫ Czarny | GND na PCF8574 **I** GND na DS3231 |

**RYSUNEK ASCII:**
```
     ESP32-C3
    ┌─────────┐
    │         │
    │  GPIO6  ├──┬────> [PCF8574 SDA]
    │  (SDA)  │  │
    │         │  └────> [DS3231 SDA]
    │         │
    │  GPIO7  ├──┬────> [PCF8574 SCL]
    │  (SCL)  │  │
    │         │  └────> [DS3231 SCL]
    │         │
    │  3.3V   ├──┬────> [PCF8574 VCC]
    │         │  │
    │         │  └────> [DS3231 VCC]
    │         │
    │  GND    ├──┬────> [PCF8574 GND]
    │         │  └────> [DS3231 GND]
    └─────────┘
```

⚠️ **UWAGA**: To jak podłączanie lampek świątecznych - łączysz je "równolegle" (obok siebie), NIE "szeregowo" (jedna za drugą)!

---

### 2.3. Podłącz klawiaturę 3x4 do PCF8574

**PCF8574 to "tłumacz"** - zamienia 7 przewodów klawiatury na 2 przewody I2C dla ESP32.

#### Klawiatura MOD-01681 ma 7 przewodów (piny 1-7 na module):

**Fizyczne piny na module klawiatury:**
- **Pin 1**: Kolumna 2 (środkowa)
- **Pin 2**: Rząd 1 (góra: 1, 2, 3)
- **Pin 3**: Kolumna 1 (lewa)
- **Pin 4**: Rząd 4 (dół: *, 0, #)
- **Pin 5**: Kolumna 3 (prawa)
- **Pin 6**: Rząd 3 (7, 8, 9)
- **Pin 7**: Rząd 2 (4, 5, 6)

#### Schemat klawiatury 3x4:

```
        Pin3  Pin1  Pin5
        (C1)  (C2)  (C3)
         │     │     │
Pin2 ─── 1     2     3    (Rząd 1)
Pin7 ─── 4     5     6    (Rząd 2)
Pin6 ─── 7     8     9    (Rząd 3)
Pin4 ─── *     0     #    (Rząd 4)
```

#### Podłącz przewody MOD-01681 → PCF8574:

| Pin modułu | Funkcja | Pin PCF8574 | Uwagi |
|------------|---------|-------------|-------|
| Pin 1 | Kolumna 2 (środek) | P4 | Przyciski: 2, 5, 8, 0 |
| Pin 2 | Rząd 1 (góra) | P0 | Przyciski: 1, 2, 3 |
| Pin 3 | Kolumna 1 (lewo) | P5 | Przyciski: 1, 4, 7, * |
| Pin 4 | Rząd 4 (dół) | P3 | Przyciski: *, 0, # |
| Pin 5 | Kolumna 3 (prawo) | P6 | Przyciski: 3, 6, 9, # |
| Pin 6 | Rząd 3 | P2 | Przyciski: 7, 8, 9 |
| Pin 7 | Rząd 2 | P1 | Przyciski: 4, 5, 6 |

💡 **TIP**: Numeracja pinów na module MOD-01681 jest od 1 do 7 (czytaj od lewej do prawej patrząc na złącza). Podłącz dokładnie według tabeli wyżej!

---

### 2.4. Podłącz czytnik RFID (RC522)

**To wykrywa karty RFID (klucze)**

RC522 używa **SPI** - szybszego połączenia niż I2C (potrzebuje 6 przewodów, opcjonalnie 7).

| Przewód RC522 | Pin ESP32 | GPIO | Co to robi? | Wymagany? |
|---------------|-----------|------|-------------|-----------|
| **SDA (SS)** | GPIO 3 (D2) | 3 | "Wybór" czytnika | ✅ TAK |
| **SCK** | GPIO 7 (D8) | 7 | Zegar (takt) | ✅ TAK |
| **MOSI** | GPIO 9 (D10) | 9 | Dane ESP→RC522 | ✅ TAK |
| **MISO** | GPIO 8 (D9) | 8 | Dane RC522→ESP | ✅ TAK |
| **RST** | GPIO 2 | 2 | Reset czytnika | ✅ TAK |
| **IRQ** | - | - | Przerwanie (interrupt) | ❌ NIE* |
| **VCC** | 3.3V | - | Zasilanie | ✅ TAK |
| **GND** | GND | - | Masa | ✅ TAK |

#### 📌 Co z pinem IRQ?

**Pin IRQ (Interrupt Request) jest OPCJONALNY!**

✅ **Bez IRQ (obecna konfiguracja - ZALECANE):**
- ESP32 sprawdza kartę co 500ms (polling)
- Prostsza konfiguracja - mniej przewodów
- Wystarczająca wydajność dla zamka
- **Zostaw IRQ niepodłączony** - działa bez niego!

⚡ **Z IRQ (zaawansowane - opcjonalne):**
- RC522 wysyła sygnał gdy wykryje kartę
- Szybsza reakcja (0ms opóźnienia)
- Niższe zużycie energii (ESP32 czeka zamiast ciągle sprawdzać)
- Wymaga dodatkowego przewodu i modyfikacji kodu

💡 **REKOMENDACJA**: Dla PineLock **nie podłączaj IRQ** - polling co 500ms w zupełności wystarczy! Opóźnienie 0.5s przy przykładaniu karty jest niezauważalne.

⚠️ **WAŻNE**: 
- Używaj **KRÓTKICH** przewodów (max 10cm) - długie przewody = problemy!
- RC522 działa na **3.3V** NIE 5V! (spalisz go!)

---
### 2.5. Podłącz zegar RTC (DS3231)

**To pamięta czas nawet gdy nie ma prądu** (dzięki baterii CR2032)

✅ **JUŻ PODŁĄCZONE!** DS3231 używa tego samego I2C co PCF8574 (z kroku 2.2)

**Dodatkowo:**
1. **Włóż** baterię CR2032 do DS3231 (okrągła płaska bateria)
2. Bateria trzyma czas gdy zabraknie zasilania
3. Zegar ustawi się automatycznie przy pierwszym uruchomieniu

---

### 2.6. Podłącz sterowanie zamkiem (MOSFET)

**MOSFET to "przełącznik elektroniczny"** - ESP32 (3.3V) steruje zamkiem (12V)

#### Dlaczego MOSFET?
ESP32 może dać **max 40mA**. Zamek potrzebuje **200-500mA**! MOSFET pozwala małym sygnałem (3.3V) włączać duży prąd (12V).

#### Schemat połączeń:

```
┌──────────┐
│  ESP32   │
│          │
│  GPIO10  ├───────> [MOSFET Signal IN]
│          │
│   GND    ├───┬───> [MOSFET GND]
└──────────┘   │
               │
┌──────────┐   │
│ Zasilacz │   │
│   12V    │   │
│          │   │
│    (+)   ├───┴───> [MOSFET VCC (5V lub 12V)]
│          │
│    (+)   ├───────> [ZAMEK (+)]
│          │
│    (-)   ├───────> [MOSFET OUT-] [ZAMEK (-)]
└──────────┘
```

| Co podłączasz | Gdzie | Wyjaśnienie |
|---------------|-------|-------------|
| ESP32 GPIO10 | MOSFET Signal IN | Sygnał sterujący (3.3V) |
| ESP32 GND | MOSFET GND | Wspólna masa |
| Zasilacz 12V (+) | ZAMEK (+) | Plus do zamka |
| ZAMEK (-) | MOSFET Drain | Minus przez MOSFET |
| MOSFET Source | Zasilacz 12V (-) | Masa 12V |

⚠️ **BEZPIECZEŃSTWO:**
- Sprawdź ile Amperów pobiera Twój zamek (zwykle 200-500mA)
- Zasilacz musi dać **min 1A** (dla bezpieczeństwa 2A)
- **WSPÓLNA MASA** - GND z ESP32 i GND z zasilacza 12V muszą być połączone!

---

### 2.7. Podłącz buzzer

**Buzzer to "głośnik alarmu"** - piszczy gdy ktoś wpisał zły PIN lub wykryto wstrząsy.

**Typ buzzera**: Aktywny buzzer 2-przewodowy 3.3V (nie pasywny!)

#### Jak podłączyć buzzer 2-przewodowy:

```
┌──────────┐
│  ESP32   │
│          │
│  GPIO 1  ├──────> [Buzzer +] (czerwony/VCC)
│          │
│   GND    ├──────> [Buzzer -] (czarny/GND)
└──────────┘
```

| Przewód buzzera | Pin ESP32 | Wyjaśnienie |
|-----------------|-----------|-------------|
| **+ (VCC/I/O)** | GPIO 1 | Plus - zasilanie sterowane |
| **- (GND)** | GND | Masa |

💡 **Jak to działa?**
- Gdy GPIO 1 = HIGH (3.3V) → buzzer dostaje zasilanie i **PISZCZY** 🔊
- Gdy GPIO 1 = LOW (0V) → buzzer bez zasilania i **CICHO** 🔇
- ESP32 może dać max 40mA z pinu - wystarczy dla małego buzzera aktywnego!

⚠️ **WAŻNE**: 
- Użyj **buzzera AKTYWNEGO** (wystarczy napięcie aby piszczał)
- **NIE** używaj buzzera pasywnego (wymaga sygnału PWM)
- Buzzer musi być **3.3V** (nie 5V!) - większe napięcie może uszkodzić ESP32
- Jeśli masz buzzer z 3 pinami (VCC, I/O, GND) - podłącz VCC do 3.3V, I/O zostaw wolny

---

### 2.8. Podłącz czujnik wstrząsów (Waveshare 9536)

**Czujnik wstrząsów wykrywa próby włamania** - gdy ktoś szarpie za zamek lub drzwi.

**Waveshare 9536 to moduł SW-420** - bardzo czuły czujnik wibracji.

| Przewód czujnika | Pin ESP32 | GPIO | Uwagi |
|------------------|-----------|------|-------|
| **VCC** | 3.3V | - | Zasilanie |
| **D0 (Digital)** | GPIO 21 (D6) | 21 | Wyjście cyfrowe (LOW = wstrząs); dzielone z TX (UART), ale dostępne jako wejście |
| **GND** | GND | - | Masa |

💡 **TIP**: 
- Czujnik ma **potencjometr** - kręć nim aby ustawić czułość
- Kręć w prawo = mniej czuły (tylko mocne uderzenia)
- Kręć w lewo = bardziej czuły (wykrywa lekkie wibracje)
- **Testuj** czułość przed montażem - nie może reagować na wiatr!

#### Co się dzieje gdy wykryje wstrząsy:
1. Czujnik wysyła sygnał LOW na pin D0
2. ESP32 wykrywa zmianę
3. **Buzzer piszczy przez 1 sekundę** ⚠️
4. ESP32 wysyła alert przez MQTT do serwera
5. Możesz zobaczyć powiadomienie w aplikacji

---

## 💻 KROK 3: Zainstaluj oprogramowanie

### 3.1. Zainstaluj PlatformIO

**PlatformIO to program do wgrywania kodu na ESP32**

#### Opcja A: Przez terminal (Linux/Mac)

```bash
pip install platformio
```

#### Opcja B: Przez VS Code (łatwiejsze!)

1. Otwórz **VS Code**
2. Kliknij **ikonę rozszerzeń** (kwadraty po lewej) lub `Ctrl+Shift+X`
3. Wyszukaj: **"PlatformIO IDE"**
4. Kliknij **Install**
5. Poczekaj ~5 minut na instalację
6. Uruchom VS Code ponownie

> ⚠️ **WAŻNE (PineLock build/upload/monitor):** Wszystkie komendy `pio ...` muszą korzystać z tego samego środowiska co serwer, bo tylko tam jest zainstalowana właściwa wersja PlatformIO. **Zawsze przed `pio run`, `pio device monitor`, itp. wykonaj w katalogu głównym repo:**
>
> ```bash
> source server/venv/bin/activate
> ```
>
> Następnie przejdź do `firmware/lock_node` i dopiero wtedy uruchamiaj polecenia. Jeśli zapomnisz aktywować `server/venv`, system użyje starego `pio` z `/usr/bin`, co kończy się błędami i konfliktami wersji.

---

### 3.2. Skonfiguruj WiFi i MQTT

#### Otwórz plik konfiguracyjny:

```bash
cd ~/PineLock/firmware/lock_node
cp include/config.h.example include/config.h
nano include/config.h
```

#### Wypełnij dane (BARDZO WAŻNE!):

```cpp
// 🌐 WiFi - TA SAMA sieć co Raspberry Pi!
#define WIFI_SSID "TwojaNazwaWiFi"         // Nazwa sieci (2.4GHz, NIE 5GHz!)
#define WIFI_PASSWORD "TwojeHasloWiFi"     // Hasło do WiFi

// 📡 MQTT - adres Raspberry Pi
#define MQTT_BROKER "192.168.1.100"         // ← TWÓJ adres IP Raspberry Pi!
#define MQTT_PORT 1883                       // Port MQTT (standard)
#define MQTT_USERNAME "pinelock"             // Użytkownik z setup Raspberry Pi
#define MQTT_PASSWORD "pinelock123"          // Hasło MQTT z Raspberry Pi

// 🔑 ID zamka - UNIKALNY dla każdego ESP32!
#define DEVICE_ID "domek_1"  // "domek_1", "domek_2", "domek_3"...
```

**Zapisz**: `Ctrl+O`, `Enter`, `Ctrl+X`

⚠️ **UWAGA**: 
- Każdy ESP32 MUSI mieć **inny DEVICE_ID**!
- ESP32 NIE obsługuje WiFi 5GHz - tylko **2.4GHz**!
- MQTT_BROKER to adres IP Raspberry Pi (ten sam co w kroku 2.3 z server/SETUP.md)

---

## 🚀 KROK 4: Wgraj kod na ESP32

### 4.1. Podłącz ESP32 do komputera

1. Weź **kabel USB-C** (musi być z DANYMI, nie tylko ładowarka!)
2. Podłącz **ESP32** do komputera
3. Sprawdź czy komputer wykrył urządzenie:

```bash
# Linux/Mac:
ls /dev/tty* | grep USB

# Powinieneś zobaczyć coś jak:
# /dev/ttyUSB0  lub  /dev/ttyACM0
```

### 4.2. Zbuduj i wgraj firmware

#### Przez terminal:

```bash
cd ~/PineLock/firmware/lock_node

# Tylko skompiluj (sprawdź błędy)
pio run

# Skompiluj i wgraj na ESP32
pio run --target upload

# Zobacz co ESP32 wysyła (logi)
pio device monitor
```

#### Przez VS Code:

1. Otwórz folder `PineLock/firmware/lock_node` w VS Code
2. Kliknij **ikonę PlatformIO** (główka mrówki) po lewej
3. Rozwiń **env:seeed_xiao_esp32c3**
4. Kliknij **Upload** (wgrywa kod)
5. Kliknij **Monitor** (pokazuje logi)

### 4.3. Co powinieneś zobaczyć (logi):

```
=== PineLock Firmware ===
Device ID: domek_1
Watchdog configured
PCF8574 initialized          ✓ Klawiatura OK
RTC initialized              ✓ Zegar OK
RFID initialized             ✓ Czytnik RFID OK
Hardware initialization complete
WARNING: No default PIN configured
Connecting to WiFi: TwojaNazwaWiFi...
..
WiFi connected!              ✓ WiFi działa!
IP address: 192.168.1.105
Attempting MQTT connection...
MQTT connected!              ✓ MQTT działa!
Subscribed to topics
Setup complete!              🎉 GOTOWE!
```

---

## 🧪 KROK 5: Testuj każdą część

### Test 1: Czy klawiatura działa?

1. Otwórz monitor: `pio device monitor`
2. **Naciśnij** dowolny przycisk na klawiaturze
3. Powinieneś zobaczyć:
   ```
   Key pressed: 1
   ```
4. Spróbuj wszystkich 12 przycisków (1-9, *, 0, #) - każdy powinien reagować

❌ **Nie działa?** → Sprawdź połączenia PCF8574 (krok 2.2 i 2.3)

💡 **Mapowanie MOD-01681**: 
- **Fizyczny układ** na klawiaturze: `1 2 3` / `4 5 6` / `7 8 9` / `* 0 #`
- **Piny klawiatury**: Pin1=Kol.środek, Pin3=Kol.lewo, Pin5=Kol.prawo
- **Piny PCF8574**: P0-P3=Rzędy(góra→dół), P4=środek, P5=lewo, P6=prawo
- Jeśli przyciski pokazują złe znaki, sprawdź dokładnie kolejność podłączenia według tabeli w kroku 2.3!

---

### Test 2: Czy RFID wykrywa karty?

1. **Przybliż** kartę RFID do czytnika (2-3 cm)
2. Powinieneś zobaczyć:
   ```
   RFID key detected in box: A1B2C3D4
   ```
3. **Oddal** kartę
4. Powinieneś zobaczyć:
   ```
   RFID key removed from box
   ```

❌ **Nie działa?** → Sprawdź połączenia SPI RC522 (krok 2.4)

---

### Test 3: Czy zegar pamięta czas?

1. W monitorze zobaczysz:
   ```
   RTC initialized
   Current time: 2025-11-22 14:30:00
   ```
2. **Odłącz** zasilanie ESP32
3. **Poczekaj** 10 sekund
4. **Podłącz** z powrotem
5. Czas powinien się zgadzać (bateria działa!)

❌ **Nie działa?** → Sprawdź baterię CR2032 w DS3231

---

### Test 4: Czy zamek się otwiera?

⚠️ **NAJPIERW** dodaj kod PIN przez panel webowy!

#### Dodaj kod PIN:
1. Otwórz: `http://ADRES_RASPBERRY_PI:8000/ui/login`
2. Zaloguj się: `admin` / `admin123`
3. Idź do: **Kody PIN**
4. Kliknij: **Dodaj nowy kod**
5. Wpisz: 
   - Kod: `1234`
   - Nazwa: `Test`
   - Domek: `domek_1`
6. Zapisz

#### Testuj zamek:
1. Na klawiaturze wpisz: `1`, `2`, `3`, `4`, `#`
2. Powinieneś usłyszeć/zobaczyć zamek się odblokowujący
3. Monitor pokaże:
   ```
   Access granted - PIN valid
   Unlocking...
   Lock state: UNLOCKED
   ```
4. Po 5 sekundach zamek się automatycznie zamknie

❌ **Nie działa?** → Sprawdź:
- Czy kod PIN jest w bazie danych?
- Czy DEVICE_ID się zgadza?
- Czy połączenia MOSFET są dobre? (krok 2.6)
- Czy zamek ma zasilanie 12V?

---

### Test 5: Czy MQTT komunikuje się z serwerem?

#### Na Raspberry Pi uruchom:

```bash
mosquitto_sub -h localhost -u pinelock -P pinelock123 -t "pinelock/#" -v
```

#### Powinieneś widzieć co minutę:

```
pinelock/domek_1/status {"state":"locked","battery":100}
pinelock/domek_1/heartbeat online
```

#### Wyślij komendę z serwera:

```bash
mosquitto_pub -h localhost -u pinelock -P pinelock123 \
  -t "pinelock/domek_1/command" \
  -m '{"action":"unlock"}'
```

Zamek powinien się otworzyć!

---

### Test 6: Czy buzzer działa?

1. W monitorze wpisz zły kod PIN (np. `9999#`)
2. Powinieneś usłyszeć **PIIIIIP** przez 1 sekundę
3. Monitor pokaże:
   ```
   PIN invalid!
   Buzzer activated
   Buzzer deactivated
   ```

❌ **Nie działa?** → Sprawdź:
- Czy buzzer jest podłączony do GPIO 1?
- Czy buzzer jest **aktywny 3.3V** (nie pasywny)?
- Czy polaryzacja jest dobra (+ do GPIO 1, - do GND)?
- Czy buzzer w ogóle działa? Podłącz + do 3.3V i - do GND na chwilę - powinien piszcz cały czas

---

### Test 7: Czy czujnik wstrząsów działa?

1. Otwórz monitor: `pio device monitor`
2. **Lekko stukaj** w czujnik lub podstawkę
3. Powinieneś zobaczyć:
   ```
   VIBRATION DETECTED!
   Buzzer activated
   Vibration alert sent
   ```
4. Buzzer powinien piszcz przez 1 sekundę

💡 **Regulacja czułości**:
- Jeśli **NIE reaguje** = kręć potencjometrem w LEWO (bardziej czuły)
- Jeśli reaguje **za często** (np. na wiatr) = kręć w PRAWO (mniej czuły)
- Testuj aż znajdziesz idealne ustawienie!

❌ **Nie działa?** → Sprawdź:
- Czy czujnik jest podłączony do GPIO 21 (D6)?
- Czy używasz pinu **D0** (cyfrowy), nie A0 (analogowy)?
- Czy zasilanie to 3.3V (nie 5V)?
- Czy na czujniku świeci LED przy wstrząsie?

---

## 🆘 CO ROBIĆ GDY COŚ NIE DZIAŁA?

### ❌ Problem: "PCF8574 not found"

**Rozwiązanie:**
1. Sprawdź przewody:
   - SDA = GPIO 6
   - SCL = GPIO 7
   - VCC = 3.3V
   - GND = GND
2. Użyj skanera I2C (kod poniżej)
3. Sprawdź czy PCF8574 ma zasilanie (multimetrem: 3.3V między VCC i GND)

**Skaner I2C:**
```bash
cd ~/PineLock/firmware/lock_node
pio device monitor
```

Powinieneś zobaczyć:
```
I2C device found at address 0x20  ← PCF8574 (klawiatura)
I2C device found at address 0x68  ← DS3231 (zegar)
```

---

### ❌ Problem: "RFID not responding"

**Rozwiązanie:**
1. Sprawdź przewody SPI:
   - MISO = GPIO 4
   - MOSI = GPIO 5
   - SCK = GPIO 8
   - SS = GPIO 3
   - RST = GPIO 2
2. **Używaj KRÓTKICH przewodów** (max 10cm!)
3. Sprawdź zasilanie: **3.3V** (NIE 5V!)
4. Spróbuj innej karty RFID
5. Sprawdź czy antena RC522 nie jest uszkodzona

---

### ❌ Problem: "WiFi connection failed"

**Rozwiązanie:**
1. Sprawdź SSID i hasło w `config.h`
2. ESP32 **NIE** obsługuje 5GHz - użyj **2.4GHz**!
3. Sprawdź czy WiFi jest w zasięgu (telefon łapie?)
4. Spróbuj wyłączyć zabezpieczenia WiFi tymczasowo (test)
5. Sprawdź czy router nie blokuje nowych urządzeń

---

### ❌ Problem: "MQTT connection failed"

**Rozwiązanie:**
1. Sprawdź adres IP Raspberry Pi:
   ```bash
   # Na Raspberry Pi:
   hostname -I
   ```
2. Sprawdź czy MQTT działa:
   ```bash
   # Na Raspberry Pi:
   sudo systemctl status mosquitto
   ```
3. Sprawdź hasło MQTT w `config.h` - musi być takie samo jak w Raspberry Pi `.env`
4. Sprawdź firewall - port 1883 musi być otwarty

---

### ❌ Problem: Zamek się nie otwiera

**Rozwiązanie:**
1. Sprawdź zasilanie 12V multimetrem
2. Sprawdź połączenie GPIO 4 (D3) → MOSFET IN
3. Sprawdź **wspólną masę** - GND z ESP32 i GND z zasilacza 12V muszą być połączone!
4. Testuj MOSFET ręcznie - podepnij 3.3V do IN, zamek powinien kliknąć
5. Sprawdź czy zamek pobiera mniej niż 1A (limit zasilacza)
6. Może zamek jest polaryzowany - spróbuj odwrócić (+) i (-)

---

## 📊 Szybka ściąga - wszystkie piny

### Podłączenia ESP32-C3:

| Co | Gdzie na ESP32 | Gdzie na module | Uwagi |
|----|----------------|-----------------|-------|
| **I2C SDA** | GPIO 5 (D4) | PCF8574 + DS3231 | Wspólna magistrala |
| **I2C SCL** | GPIO 6 (D5) | PCF8574 + DS3231 | Wspólna magistrala |
| **RFID SS** | GPIO 3 (D2) | RC522 SDA | Chip Select |
| **RFID MISO** | GPIO 8 (D9) | RC522 MISO | Dane RC522→ESP |
| **RFID MOSI** | GPIO 9 (D10) | RC522 MOSI | Dane ESP→RC522 |
| **RFID SCK** | GPIO 7 (D8) | RC522 SCK | Zegar SPI |
| **RFID RST** | GPIO 2 | RC522 RST | Reset czytnika |
| **Zamek** | GPIO 4 (D3) | MOSFET IN | Sterowanie zamkiem |
| **Buzzer** | GPIO 1 | Buzzer I/O | Dźwięk alarmu |
| **Czujnik wstrząsów** | GPIO 21 (D6) | Waveshare 9536 D0 | Wykrywanie wibracji |

### Podłączenia klawiatury MOD-01681 (3x4):

**Mapowanie fizycznych pinów modułu → PCF8574:**

| Pin modułu | Funkcja klawiatury | Pin PCF8574 | Przyciski |
|------------|-------------------|-------------|-----------|
| Pin 1 | Kolumna 2 (środek) | P4 | 2, 5, 8, 0 |
| Pin 2 | Rząd 1 (góra) | P0 | 1, 2, 3 |
| Pin 3 | Kolumna 1 (lewo) | P5 | 1, 4, 7, * |
| Pin 4 | Rząd 4 (dół) | P3 | *, 0, # |
| Pin 5 | Kolumna 3 (prawo) | P6 | 3, 6, 9, # |
| Pin 6 | Rząd 3 | P2 | 7, 8, 9 |
| Pin 7 | Rząd 2 | P1 | 4, 5, 6 |

💡 **Uwaga**: Kolejność kolumn na PCF8574 to P4(środek), P5(lewo), P6(prawo) - nie P4-P5-P6 lewo-środek-prawo!

---

## 🎉 Gratulacje!

Jeśli dotarłeś tutaj i wszystkie testy przeszły - **masz działający zamek ESP32**!

### Co teraz możesz zrobić:

✅ Dodać więcej kodów PIN przez panel  
✅ Zarejestrować karty RFID  
✅ Sprawdzać historię dostępu  
✅ Zbudować kolejne ESP32 dla innych domków  
✅ Umieścić wszystko w obudowie  

### Następne kroki:

1. **Zabezpiecz** - zmień domyślne hasła
2. **Testuj** - sprawdź zasięg WiFi na miejscu montażu
3. **Montuj** - umieść ESP32 w wodoodpornej obudowie
4. **Dokumentuj** - zapisz który ESP32 to który domek

---

## 🔋 Zużycie prądu (ile pobiera)

| Urządzenie | Napięcie | Prąd | Uwagi |
|------------|----------|------|-------|
| ESP32-C3 | 3.3V | ~200mA | Gdy WiFi aktywne |
| RC522 | 3.3V | ~13-26mA | Gdy czyta kartę |
| PCF8574 | 3.3V | ~1mA | Prawie nic |
| DS3231 | 3.3V | ~0.2mA | Z baterii μA |
| Buzzer | 3.3V | ~20-30mA | Gdy piszczy |
| Czujnik wstrząsów | 3.3V | ~15mA | Cały czas aktywny |
| Zamek | 12V | 200-500mA | Gdy otwarty |

**Zalecane zasilanie:**
- **12V 2A** zasilacz
- **Konwerter** 12V → 5V → USB-C do ESP32
- ESP32 ma wbudowany regulator 5V → 3.3V

---

**Powodzenia! 🌲🔐**

---

## 🚀 Zaawansowane: Użycie pinu IRQ w RC522 (opcjonalne)

**Dla zaawansowanych użytkowników, którzy chcą maksymalną wydajność i niższe zużycie energii.**

### Dlaczego IRQ?

✅ **Zalety:**
- Natychmiastowa detekcja karty (0ms zamiast do 500ms)
- Niższe zużycie energii (~10-15mA oszczędności)
- ESP32 czeka na przerwanie zamiast ciągłego sprawdzania

❌ **Wady:**
- Dodatkowy przewód
- Bardziej skomplikowany kod
- Wymaga wolnego pinu GPIO

### Krok 1: Podłącz pin IRQ

| Przewód RC522 | Pin ESP32 | GPIO |
|---------------|-----------|------|
| **IRQ** | GPIO 0 | 0 |

⚠️ **UWAGA**: GPIO 0 to pin BOOT - upewnij się że nie jest używany podczas wgrywania kodu!

### Krok 2: Dodaj do config.h

```cpp
// SPI for RC522 RFID
#define RFID_SS_PIN 3
#define RFID_RST_PIN 2
#define RFID_MISO_PIN 4
#define RFID_MOSI_PIN 5
#define RFID_SCK_PIN 8
#define RFID_IRQ_PIN 0  // Dodaj tę linię
```

### Krok 3: Modyfikacja main.cpp

W sekcji setup dodaj konfigurację IRQ:

```cpp
// Initialize RFID with custom SPI pins
SPI.begin(RFID_SCK_PIN, RFID_MISO_PIN, RFID_MOSI_PIN, RFID_SS_PIN);
rfid.PCD_Init();

// Configure IRQ pin
pinMode(RFID_IRQ_PIN, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(RFID_IRQ_PIN), rfidISR, FALLING);

// Enable IRQ in RC522
rfid.PCD_WriteRegister(rfid.ComIEnReg, 0xA0); // Enable RxIRq and IdleIRq
```

Dodaj funkcję obsługi przerwania:

```cpp
volatile bool rfidCardDetected = false;

void IRAM_ATTR rfidISR() {
    rfidCardDetected = true;
}
```

Zmodyfikuj `handleRFID()`:

```cpp
void handleRFID() {
    if (!rfidCardDetected) {
        return; // Czekaj na przerwanie
    }
    
    rfidCardDetected = false;
    
    // Reszta kodu jak poprzednio...
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        // ...
    }
}
```

💡 **TIP**: To zaawansowana modyfikacja! Jeśli nie jesteś pewien - zostań przy polling (obecna konfiguracja).

---

### 2️⃣ PCF8574 I2C Expander (Keypad Interface)

**4x4 Matrix Keypad Connection**

| PCF8574 Pin | Function | Keypad Pin |
|-------------|----------|------------|
| P0 | Row 1 | R1 |
| P1 | Row 2 | R2 |
| P2 | Row 3 | R3 |
| P3 | Row 4 | R4 |
| P4 | Column 1 | C1 |
| P5 | Column 2 | C2 |
| P6 | Column 3 | C3 |
| P7 | Column 4 | C4 |

**I2C Address:** `0x20` (default)

<details>
<summary>🔍 <b>Keypad Layout Reference</b></summary>

```
┌───┬───┬───┬───┐
│ 1 │ 2 │ 3 │ A │
├───┼───┼───┼───┤
│ 4 │ 5 │ 6 │ B │
├───┼───┼───┼───┤
│ 7 │ 8 │ 9 │ C │
├───┼───┼───┼───┤
│ * │ 0 │ # │ D │
└───┴───┴───┴───┘

* = Clear PIN
# = Submit PIN
A-D = Reserved
```

</details>

---

### 3️⃣ RC522 RFID Reader (Key Presence Detection)

**SPI Connection to ESP32-C3**

| RC522 Pin | ESP32-C3 Pin | GPIO | Description |
|-----------|--------------|------|-------------|
| SDA (SS) | GPIO 3 | 3 | Chip Select |
| SCK | GPIO 8 | 8 | SPI Clock |
| MOSI | GPIO 5 | 5 | Master Out |
| MISO | GPIO 4 | 4 | Master In |
| IRQ | - | - | Not connected |
| GND | GND | - | Ground |
| RST | GPIO 2 | 2 | Reset |
| VCC | 3.3V | - | Power |

> 💡 **Tip**: Use short wires for SPI connections to avoid signal integrity issues.

---

### 4️⃣ DS3231 RTC Module

**Real-Time Clock with Battery Backup**

Already connected via I2C bus (shared with PCF8574)

**Additional Setup:**
1. Insert CR2032 battery into battery holder
2. Set initial time on first boot (auto-set to compile time)
3. RTC will maintain time during power loss

```cpp
// Time is automatically set on first boot:
rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
```

---

### 5️⃣ MOSFET Lock Control

**12V Electromagnetic Lock Driver**

| Connection | Pin/Wire | Notes |
|------------|----------|-------|
| Signal IN | GPIO 4 (D3) | From ESP32-C3 |
| VCC | 5V | MOSFET module power |
| GND | Common GND | Shared ground |
| MOSFET OUT+ | Lock + | To lock positive |
| MOSFET OUT- | 12V GND | To lock negative |

**Power Circuit:**
```
12V PSU (+) ──→ Lock (+)
Lock (-)    ──→ MOSFET Drain
MOSFET Source ──→ 12V PSU (-)
ESP32 GPIO4 (D3) ──→ MOSFET Gate (via module)
```

> 🔒 **Safety**: Ensure proper current rating for your lock. Typical locks draw 200-500mA.

---

## 💻 Software Setup

### 1️⃣ Install PlatformIO

**Option A: Command Line**
```bash
pip install platformio
```

**Option B: VS Code Extension**
1. Open VS Code
2. Go to Extensions (Ctrl+Shift+X)
3. Search for "PlatformIO IDE"
4. Click Install

---

### 2️⃣ Configure WiFi and MQTT

Edit `include/config.h`:

Edit `include/config.h`:

```cpp
// 🌐 WiFi Configuration
#define WIFI_SSID "YourWiFiNetwork"       // Your 2.4GHz network name
#define WIFI_PASSWORD "YourWiFiPassword"  // Network password

// 📡 MQTT Broker Configuration
#define MQTT_BROKER "192.168.1.100"       // Broker IP (e.g., Raspberry Pi)
#define MQTT_PORT 1883                     // Standard MQTT port
#define MQTT_USERNAME "pinelock"           // MQTT username (or "" if none)
#define MQTT_PASSWORD "your_mqtt_password" // MQTT password (or "" if none)

// 🔑 Device Configuration
#define DEVICE_ID "lock_001"  // ⚠️ MUST BE UNIQUE FOR EACH LOCK!
```

> 🔐 **Security Reminder**: Never commit real credentials to Git. Use `config.h.example` as template.

---

### 3️⃣ Build & Upload Firmware

**Using PlatformIO CLI:**
```bash
cd lock_node
pio run                    # Build only
pio run --target upload    # Build and upload
pio device monitor         # View serial output
```

**Using VS Code:**
1. Open `lock_node` folder in VS Code
2. Click PlatformIO icon in sidebar
3. Select "Upload" under env:esp32-c3
4. Click "Monitor" to view output

---

## 🧪 Testing & Verification

### Initial Boot Sequence

Expected serial output:
```
=== PineLock Firmware ===
Device ID: lock_001
Watchdog configured
PCF8574 initialized
RTC initialized
RFID initialized
Hardware initialization complete
WARNING: No default PIN configured. Add PINs via MQTT.
WiFi connected!
IP address: 192.168.1.XXX
MQTT connected!
Subscribed to topics
```

### Component Testing

#### ✅ Test 1: I2C Devices
```bash
# Both PCF8574 and DS3231 should be detected
# Check serial output for "PCF8574 initialized" and "RTC initialized"
```

#### ✅ Test 2: Keypad
```bash
# Press any key on keypad
# Should see: "Key pressed: X" in serial monitor
```

#### ✅ Test 3: RFID Reader
```bash
# Place RFID card near reader
# Should see: "RFID key detected in box: XXXXXXXX"
```

#### ✅ Test 4: Lock Control
```bash
# Via MQTT:
mosquitto_pub -h localhost -t "pinelock/lock_001/command" -m '{"action":"unlock"}'
# Lock should activate for 5 seconds then re-lock
```

#### ✅ Test 5: MQTT Communication
```bash
# Subscribe to all device topics:
mosquitto_sub -h localhost -t "pinelock/lock_001/#" -v

# You should see:
# - Heartbeat every 60 seconds
# - Status updates on state changes
# - Access events when PIN entered
```

---

## 🐛 Troubleshooting

<details>
<summary><b>❌ "PCF8574 not found" Error</b></summary>

**Solutions:**
1. Check I2C wiring (SDA=GPIO6, SCL=GPIO7)
2. Verify 3.3V power to PCF8574
3. Check I2C address (default 0x20)
4. Use I2C scanner to detect device:
   ```cpp
   Wire.begin(6, 7);
   for(byte i = 1; i < 127; i++) {
       Wire.beginTransmission(i);
       if(Wire.endTransmission() == 0) {
           Serial.printf("Found I2C device at 0x%02X\n", i);
       }
   }
   ```

</details>

<details>
<summary><b>❌ "RTC not found" Error</b></summary>

**Solutions:**
1. Verify I2C connections (shared with PCF8574)
2. Check CR2032 battery installation
3. Verify DS3231 module power (3.3V)
4. Check for I2C address conflicts

</details>

<details>
<summary><b>❌ RFID Not Detecting Cards</b></summary>

**Solutions:**
1. Verify SPI connections:
   - MISO = GPIO 4
   - MOSI = GPIO 5
   - SCK = GPIO 8
   - SS = GPIO 3
   - RST = GPIO 2
2. Check 3.3V power supply
3. Use shorter wires (max 10cm for SPI)
4. Test with MFRC522 library examples
5. Try different RFID cards

</details>

<details>
<summary><b>❌ WiFi Connection Failed</b></summary>

**Solutions:**
1. Verify SSID and password in config.h
2. Ensure 2.4GHz network (ESP32-C3 doesn't support 5GHz)
3. Check WiFi signal strength
4. Disable WiFi security temporarily for testing
5. Check for MAC address filtering on router

</details>

<details>
<summary><b>❌ MQTT Connection Failed</b></summary>

**Solutions:**
1. Verify broker IP address
2. Test broker: `mosquitto_sub -h [broker_ip] -t test`
3. Check firewall settings
4. Verify username/password (or use empty strings)
5. Ensure broker allows remote connections

</details>

<details>
<summary><b>❌ Watchdog Reset Loop</b></summary>

**Solutions:**
1. Check for blocking code in loop()
2. Increase WDT_TIMEOUT from 30 to 60 seconds
3. Ensure WiFi/MQTT don't block indefinitely
4. Add debug prints to locate hang point

</details>

<details>
<summary><b>❌ Lock Not Activating</b></summary>

**Solutions:**
1. Verify GPIO 4 (D3) connection to MOSFET
2. Check 12V power supply
3. Test MOSFET manually with 3.3V signal
4. Verify lock polarity (some locks are polarized)
5. Check current rating (lock may draw more than supply provides)

</details>

---

## 📊 Pin Summary Table

| Function | GPIO | Component | Notes |
|----------|------|-----------|-------|
| I2C SDA | 6 | PCF8574, DS3231 | Shared bus |
| I2C SCL | 7 | PCF8574, DS3231 | Shared bus |
| SPI SS | 3 | RC522 | Chip select |
| SPI MISO | 4 | RC522 | Data in |
| SPI MOSI | 5 | RC522 | Data out |
| SPI SCK | 8 | RC522 | Clock |
| RFID RST | 2 | RC522 | Reset |
| Lock Control | 10 | MOSFET | PWM capable |

---

## 🔋 Power Requirements

| Component | Voltage | Current | Notes |
|-----------|---------|---------|-------|
| ESP32-C3 | 3.3V | ~200mA | Via USB or regulator |
| PCF8574 | 3.3V | ~1mA | Low power |
| DS3231 | 3.3V | ~0.2mA | Battery backup |
| RC522 | 3.3V | ~13-26mA | During read |
| Lock | 12V | 200-500mA | Check your model |
| **Total** | **12V** | **~1A** | **Recommended: 2A supply** |

**Recommended Setup:**
- 12V 2A power supply
- 12V → Buck converter → 5V → ESP32-C3 (3.3V regulated onboard)
- 12V → Direct to lock via MOSFET

---

## 🎯 Next Steps

After successful hardware setup:

1. 📖 Read [DEPLOYMENT.md](DEPLOYMENT.md) for production deployment
2. 🧪 Run full test suite (see DEPLOYMENT.md)
3. 🔐 Add PIN codes and RFID cards
4. 📊 Configure server/backend for MQTT
5. 🏠 Install in final location

---

<div align="center">

### 🌲 PineLock Hardware Setup Complete!

**Ready for firmware deployment** ✅

[← Back to README](README.md) | [Deployment Guide →](DEPLOYMENT.md)

</div>

```bash
pio run --target upload
```

### 5. Monitor Serial Output

```bash
pio device monitor
```

Expected output:
```
=== PineLock Firmware ===
Device ID: lock_001
PCF8574 initialized
RTC initialized
RFID initialized
Hardware initialization complete
Connecting to WiFi: YourWiFiNetwork
...
WiFi connected!
IP address: 192.168.1.XXX
Attempting MQTT connection...connected!
Subscribed to topics
Setup complete!
```

## Testing

### 1. Test Default PIN

The firmware includes a default PIN code `1234` for testing:

1. Enter `1234#` on the keypad
2. Lock should unlock
3. Check serial monitor for confirmation
4. Lock should auto-lock after 5 seconds

### 2. Test RFID Key Detection

1. Register an RFID key in the server database first
2. Place the registered key near the RFID reader
3. Serial monitor should show "Valid key present in box"
4. Server should receive MQTT message with key presence status
5. Remove the key - should show "RFID key removed from box"

### 3. Test MQTT Commands

Using mosquitto_pub:

```bash
# Unlock
mosquitto_pub -h localhost -t "pinelock/lock_001/command" \
  -m '{"action":"unlock"}'

# Lock
mosquitto_pub -h localhost -t "pinelock/lock_001/command" \
  -m '{"action":"lock"}'
```

### 4. Monitor MQTT Messages

```bash
mosquitto_sub -h localhost -t "pinelock/#" -v
```

You should see:
- Heartbeat messages every 60 seconds
- Status updates when lock state changes
- Access events when PIN/RFID is used

## I2C Scanner (Troubleshooting)

If devices aren't detected, use this I2C scanner sketch:

```cpp
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(6, 7);  // SDA, SCL
  
  Serial.println("\nI2C Scanner");
  
  for(byte address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
    }
  }
}

void loop() {}
```

Expected devices:
- 0x20: PCF8574 (keypad)
- 0x68: DS3231 (RTC)

## Pin Configuration Quick Reference

```
ESP32-C3 Pin Mapping:
GPIO 2  → RC522 RST
GPIO 3  → RC522 SS
GPIO 4  → RC522 MISO
GPIO 5  → RC522 MOSI
GPIO 6  → I2C SDA (PCF8574, DS3231)
GPIO 7  → I2C SCL (PCF8574, DS3231)
GPIO 8  → RC522 SCK
GPIO 4 (D3) → MOSFET Gate (Lock Control)
```

## Troubleshooting

### WiFi Not Connecting
- Check SSID and password in config.h
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check WiFi signal strength

### MQTT Not Connecting
- Verify broker IP address
- Check MQTT credentials
- Ensure mosquitto is running on server
- Check firewall rules

### PCF8574 Not Found
- Verify I2C connections (SDA, SCL)
- Check I2C address with scanner
- Try different I2C address (0x20-0x27)

### RC522 Not Detecting Keys
- Check SPI connections
- Verify 3.3V power (not 5V!)
- Try different RFID cards
- Check RST and SS pin definitions

### RTC Issues
- Insert CR2032 battery
- Check I2C connections
- Time will be set to compile time on first boot

### Lock Not Operating
- Check MOSFET connections
- Verify 12V power supply
- Test MOSFET with multimeter
- Ensure common ground between ESP32 and 12V supply

## OTA Updates (Future Enhancement)

For remote firmware updates, add OTA support:

```cpp
#include <ArduinoOTA.h>

void setup() {
  // ... existing setup ...
  
  ArduinoOTA.setHostname(DEVICE_ID);
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  // ... existing loop ...
}
```

## Power Considerations

- ESP32-C3: ~80mA @ 3.3V (WiFi active)
- RC522: ~50mA @ 3.3V (active)
- PCF8574: ~1mA @ 3.3V
- DS3231: ~200μA @ 3.3V
- Lock: ~500mA @ 12V (when unlocked)

Recommended power supply:
- 5V 2A for ESP32 and modules
- 12V 1A for electromagnetic lock
- Separate power domains with common ground
