# Credits and attribution

## Chess engine

Search and rules descend from **Sergey Urusov**’s ESP32 chess engine
([Hackster project](https://www.hackster.io/Sergey_Urusov/esp32-chess-engine-c29dd9),
originally published under **GPL-3+**).

Packaged for PlatformIO by **hpsaturn**:
[hpsaturn/esp32-chess-engine](https://github.com/hpsaturn/esp32-chess-engine).

This repository (`esp32-chess-lib` / the `components/chess` submodule) ports that
engine to a **C ABI** (`chess_api`) with host tests, benches, and ESP-IDF
integration. Engine source remains **GPL-3.0** — see [`LICENSE`](LICENSE).

Do not remove the author credit in `src/chess_engine.cpp` when redistributing.

## This firmware application

UI, board HAL, touch calibration, and glue in this tree are part of the same
**GPL-3.0** combined work when linked with the engine.

## Third-party

| Component | License | Notes |
|---|---|---|
| ESP-IDF / `esp_lcd` | Apache-2.0 | Compatible with GPL-3 when combined |
| Win At Chess (WAC) EPD FENs | Public test suite | Same positions as commonly redistributed WAC EPDs (e.g. Arasan `wacnew.epd`); used for regression only |

## Corresponding source

Public Git repositories are the preferred corresponding source for any binary
firmware or library builds you distribute. Prefer tagging releases so a binary
maps to an exact commit.
