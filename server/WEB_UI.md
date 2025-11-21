# 🌲 PineLock Web UI

Nowoczesny interfejs użytkownika dla systemu PineLock zaprojektowany zgodnie z brandingiem projektu.

## 🎨 Design

UI został zaprojektowany w oparciu o logo PineLock i wytyczne brandingowe:

- **Kolory**: Leśna zieleń (#1e5945, #2d6f51, #3a8660)
- **Motyw**: Choinka z dziurką od klucza
- **Styl**: Nowoczesny, minimalistyczny, responsywny

## 📄 Strony

### 🔐 Logowanie (`/ui/login`)
- Formularz logowania z logo PineLock
- Domyślne dane logowania: `admin` / `admin` (zmień w konfiguracji!)
- Responsywny design z animacjami

### 🏠 Dashboard (`/ui/dashboard`)
- Przegląd wszystkich domków
- Statystyki: całkowita liczba, zamknięte, otwarte, offline
- Karty domków z możliwością szybkiego zamykania/otwierania
- Historia ostatnich zdarzeń
- Auto-odświeżanie co 30 sekund

### 🔐 Szczegóły domku (`/ui/locks/{id}`)
- Status urządzenia (online/offline, zamknięte/otwarte)
- Lista metod dostępu (PIN, RFID)
- Historia dostępu
- Możliwość zdalnego sterowania zamkiem
- Strefa zagrożenia z opcją usunięcia domku

### ➕ Dodaj domek (`/ui/locks/new`)
- Formularz dodawania nowego domku
- Walidacja Device ID
- Instrukcje konfiguracji firmware
- Wskazówki dotyczące dalszych kroków

## 🚀 Uruchomienie

### Wymagania
```bash
pip install -r requirements.txt
```

### Migracja bazy danych (jeśli masz istniejącą bazę)
```bash
python3 migrate_db.py
```

### Start serwera
```bash
./start.sh
```

UI będzie dostępne pod adresem: `http://localhost:8000/ui/login`

## 🔧 Konfiguracja

W pliku `.env` lub zmiennych środowiskowych:

```bash
# Dane logowania do UI
ADMIN_USERNAME=admin
ADMIN_PASSWORD=zmien_to_haslo

# Klucz sesji (WAŻNE: zmień w produkcji!)
SESSION_SECRET_KEY=zmien_ten_klucz_na_losowy
```

Generowanie bezpiecznego klucza sesji:
```bash
python3 -c "import secrets; print(secrets.token_hex(32))"
```

## 🎯 Funkcje

### ✅ Zaimplementowane
- [x] Strona logowania z brandingiem
- [x] Dashboard z kartami domków
- [x] Statystyki w czasie rzeczywistym
- [x] Szczegóły domku
- [x] Lista metod dostępu
- [x] Zdalne sterowanie zamkami
- [x] Dodawanie nowych domków
- [x] Responsywny design
- [x] Auto-odświeżanie
- [x] Animacje i przejścia

### 🚧 W przygotowaniu
- [ ] Historia dostępu (pełna integracja z logami)
- [ ] Dodawanie metod dostępu przez UI
- [ ] Edycja domków
- [ ] Filtry i wyszukiwanie
- [ ] Eksport danych
- [ ] Zarządzanie użytkownikami
- [ ] Powiadomienia push

## 🎨 Komponenty UI

### Kolory
```css
--forest-green: #1e5945;    /* Główny kolor brandingowy */
--dark-green: #2d6f51;      /* Ciemniejszy odcień */
--pine-green: #3a8660;      /* Akcenty */
--cream: #f5f5dc;           /* Tło */
--white: #ffffff;           /* Karty */
```

### Ikony
- 🏠 Panel główny
- 🔐 Domki
- 🔢 Kody PIN
- 📇 Karty RFID
- 📊 Historia
- ⚙️ Ustawienia

### Komponenty
- **Lock Card**: Karta domku z statusem i akcjami
- **Stat Card**: Karta statystyk
- **Alert**: Powiadomienia (sukces, błąd, info)
- **Modal**: Okna modalne
- **Form**: Formularze z walidacją
- **Table**: Tabele danych
- **Sidebar**: Boczne menu nawigacji

## 📱 Responsive Design

UI automatycznie dostosowuje się do różnych rozmiarów ekranu:

- **Desktop** (>768px): Pełny layout z bocznym menu
- **Mobile** (<768px): Menu górne, pojedyncza kolumna

## 🔒 Bezpieczeństwo

- Sesje z secure cookies
- CSRF protection (wbudowany w FastAPI)
- Walidacja danych wejściowych
- Bezpieczne hasła (zmień domyślne!)

## 🛠️ Rozwój

### Struktura plików
```
server/
├── app/
│   ├── templates/          # Szablony Jinja2
│   │   ├── base.html      # Szablon bazowy
│   │   ├── login.html     # Strona logowania
│   │   ├── dashboard.html # Dashboard
│   │   ├── lock_detail.html
│   │   └── lock_new.html
│   ├── static/            # Pliki statyczne
│   │   └── styles.css     # Style CSS
│   ├── ui_routes.py       # Trasy UI
│   └── ...
└── migrate_db.py          # Skrypt migracji
```

### Dodawanie nowych stron

1. Utwórz template w `templates/`
2. Dodaj trasę w `ui_routes.py`
3. Zaktualizuj menu w sidebar (jeśli potrzeba)

### Modyfikacja stylów

Edytuj `static/styles.css` - wszystkie zmienne CSS są zdefiniowane w `:root`

## 📚 Dokumentacja

- [AGENTS.md](/AGENTS.md) - Wytyczne dla agentów AI
- [BRANDING.md](/firmware/lock_node/BRANDING.md) - Wytyczne brandingowe
- [API_EXAMPLES.md](/API_EXAMPLES.md) - Przykłady API

## 🐛 Rozwiązywanie problemów

### Brak logo/stylów
- Sprawdź, czy `static/` folder istnieje i zawiera `styles.css`
- Sprawdź logi serwera pod kątem błędów 404

### Błąd sesji
- Upewnij się, że `SESSION_SECRET_KEY` jest ustawiony
- Wyczyść cookies przeglądarki

### Baza danych
- Uruchom `migrate_db.py` jeśli masz starą bazę
- Usuń `pinelock.db` aby zacząć od nowa (UWAGA: usunie dane!)

## 📸 Screenshoty

*(Dodaj screenshoty po wdrożeniu)*

## 🌲 Credits

Projekt PineLock © 2025
Design oparty na logo PineLock (choinka z dziurką od klucza)
