# 🌲 PineLock - Smart Lock Management System 🔐

System zarządzania zamkami elektronicznymi oparty o ESP32, FastAPI i MQTT.

## 🚀 Quick Start

### Uruchomienie serwera

```bash
cd server
./start.sh
```

Serwer będzie dostępny pod adresem: `http://localhost:8000/ui/login`

**Domyślne dane logowania:**
- Login: `admin`
- Hasło: `wkswks12`

### Konfiguracja publicznego dostępu (Tailscale Funnel)

Aby udostępnić PineLock publicznie przez internet:

```bash
cd server
./tailscale_setup.sh
```

Szczegóły w [TAILSCALE.md](TAILSCALE.md)

### Auto-start na Raspberry Pi

Aby PineLock uruchamiał się automatycznie przy starcie Raspberry Pi:

```bash
cd server
./setup_autostart.sh
```

## 📁 Struktura projektu

```
PineLock/
├── server/              # Backend FastAPI
│   ├── app/            # Kod aplikacji
│   ├── start.sh        # Skrypt uruchomieniowy
│   ├── tailscale_setup.sh       # Setup Tailscale Funnel
│   ├── setup_autostart.sh       # Setup auto-start
│   ├── pinelock.service         # Systemd service
│   └── tailscale-funnel.service # Systemd service
├── firmware/           # Firmware ESP32
└── docs/              # Dokumentacja
```

## 🔧 Funkcje

- ✅ Zdalne otwieranie/zamykanie zamków
- ✅ Zarządzanie kodami PIN
- ✅ Historia dostępu i logi
- ✅ Panel administracyjny
- ✅ Komunikacja MQTT z ESP32
- ✅ Publiczny dostęp przez Tailscale Funnel
- ✅ Auto-start na Raspberry Pi
- 🔄 Obsługa kart RFID (w przygotowaniu)

## 📚 Dokumentacja

- [TAILSCALE.md](TAILSCALE.md) - Konfiguracja publicznego dostępu
- [server/README.md](server/README.md) - Dokumentacja serwera
- [firmware/README.md](firmware/README.md) - Dokumentacja firmware

## 🛠️ Technologie

- **Backend:** FastAPI + SQLite + MQTT
- **Frontend:** Jinja2 Templates + CSS
- **Firmware:** ESP32 + PlatformIO
- **Networking:** Tailscale Funnel (HTTPS)

## 📝 Licencja

MIT License
