# 🚀 PineLock Web UI - Szybki start

## Instalacja w 5 krokach

### 1. Przejdź do katalogu serwera
```bash
cd /home/kmush/Desktop/Work/Other_repo/PineLock/server
```

### 2. Zainstaluj zależności Python
```bash
pip install -r requirements.txt
```

### 3. (Opcjonalnie) Migruj bazę danych
Jeśli masz już istniejącą bazę danych:
```bash
python3 migrate_db.py
```

### 4. Uruchom serwer
```bash
./start.sh
```

### 5. Otwórz przeglądarkę
Przejdź do: **http://localhost:8000/ui/login**

**Dane logowania:**
- Login: `admin`
- Hasło: `admin`

## 🎨 Podgląd UI

Aby zobaczyć komponenty UI bez uruchamiania serwera:
```bash
# Otwórz w przeglądarce:
file:///home/kmush/Desktop/Work/Other_repo/PineLock/server/static/preview.html
```

## ⚙️ Konfiguracja (opcjonalna)

### Zmiana hasła
Utwórz plik `.env` w katalogu `server/`:
```bash
ADMIN_USERNAME=admin
ADMIN_PASSWORD=twoje_nowe_haslo
SESSION_SECRET_KEY=wygeneruj_losowy_klucz
```

Generowanie bezpiecznego klucza:
```bash
python3 -c "import secrets; print(secrets.token_hex(32))"
```

## 📱 Funkcje UI

Po zalogowaniu będziesz mógł:
- ✅ Przeglądać wszystkie domki
- ✅ Zobaczyć statystyki (zamknięte/otwarte/offline)
- ✅ Zdalnie zamykać/otwierać domki
- ✅ Dodawać nowe domki
- ✅ Zarządzać kodami PIN i kartami RFID
- ✅ Przeglądać historię dostępu

## 🔧 Rozwiązywanie problemów

### Błąd: ModuleNotFoundError
```bash
pip install -r requirements.txt
```

### Błąd: Port 8000 zajęty
```bash
# Zabij proces na porcie 8000
lsof -ti:8000 | xargs kill -9

# Lub użyj innego portu
uvicorn app.main:app --host 0.0.0.0 --port 8001
```

### Nie działa logowanie
```bash
# Sprawdź czy SESSION_SECRET_KEY jest ustawiony
grep SESSION_SECRET_KEY .env

# Jeśli nie, dodaj:
echo "SESSION_SECRET_KEY=$(python3 -c 'import secrets; print(secrets.token_hex(32))')" >> .env
```

### Brak stylów/logo
```bash
# Sprawdź czy istnieje folder static
ls -la static/

# Jeśli nie, utwórz:
mkdir -p static
```

## 📚 Więcej informacji

- **Pełna dokumentacja UI**: [WEB_UI.md](WEB_UI.md)
- **Dokumentacja API**: http://localhost:8000/docs (po uruchomieniu serwera)
- **Wytyczne projektowe**: [../firmware/lock_node/BRANDING.md](../firmware/lock_node/BRANDING.md)

## 🌲 Enjoy PineLock!

*Secure your space. Protect what matters.*
