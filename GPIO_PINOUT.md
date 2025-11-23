# Seeed XIAO ESP32C3 - Mapowanie GPIO

## Tabela Pinów (Standard Xiao ESP32C3)

|   D-pin | Analog | Funkcja dodatkowa | GPIO                                |
| ------: | :----: | :---------------- | :---------------------------------- |
|  **D0** |   A0   | **Zamek (MOSFET)**| **GPIO2** *(pull-down podczas boot)*|
|  **D1** |   A1   | RFID RST          | **GPIO3**                           |
|  **D2** |   A2   | RFID SS           | **GPIO4**                           |
|  **D3** |   A3   | —                 | **GPIO5**                           |
|  **D4** |   A4   | **I2C SDA**       | **GPIO6**                           |
|  **D5** |   A5   | **I2C SCL**       | **GPIO7**                           |
|  **D6** |    —   | Buzzer            | **GPIO21**                          |
|  **D7** |    —   | **RX** (UART)     | **GPIO20**                          |
|  **D8** |   A8   | **SPI SCK**       | **GPIO8**                           |
|  **D9** |   A9   | **SPI MISO**      | **GPIO9**                           |
| **D10** |   A10  | **SPI MOSI**      | **GPIO10**                          |

## Konfiguracja w PineLock

### I2C (Klawiatura + RTC)
- **SDA**: D4 (GPIO 6)
- **SCL**: D5 (GPIO 7)

### SPI (RFID - RC522)
- **SS**: D2 (GPIO 4)
- **RST**: D1 (GPIO 3)
- **SCK**: D8 (GPIO 8)
- **MISO**: D9 (GPIO 9)
- **MOSI**: D10 (GPIO 10)

### Wyjścia Sterujące
- **Zamek (MOSFET)**: D0 (GPIO 2) - *Ma pull-down podczas boot - bezpieczne*
- **Buzzer**: D6 (GPIO 21)

### Wejścia
- **Czujnik Wstrząsów**: *Nieużywany* - *Opcjonalnie*


---

## Uwagi
1. **Zmiana sprzętowa**: Zamek został przeniesiony z D10 na D3, aby zwolnić pin MOSI dla RFID.
2. **Bezpieczeństwo**: Zamek przeniesiony z D6 (GPIO21) na D0 (GPIO2), ponieważ GPIO2 ma sprzętowy pull-down podczas boot ESP32 - zapobiega to przypadkowemu otwarciu zamka podczas resetu/upload firmware.
3. **Klawiatura**: Adres I2C domyślnie `0x20`.
