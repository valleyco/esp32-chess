# Board support

HAL and docs are organized so more than one MCU/panel can coexist.

| Board doc | Status |
|---|---|
| [`esp32-2432s028r/`](esp32-2432s028r/BOARD.md) | Primary bring-up (classic ESP32, ST7789, XPT2046) |
| ESP32-C3 + SPI TFT | Planned |
| ILI9341 panels | Planned (verify controller — many “ILI9341” CYD units are ST7789) |

Add a short folder under `docs/boards/<id>/` with pins, SPI hosts, and display
quirks when you bring up a new variant.
