# ESP32 Chess on CYD

Playable chess on the **ESP32-2432S028R** (Cheap Yellow Display): ST7789 320×240
LCD + XPT2046 resistive touch. You play White; the engine replies on a FreeRTOS
worker task.

Living plan: [`PLAN.md`](PLAN.md). Board notes:
[`docs/boards/esp32-2432s028r/BOARD.md`](docs/boards/esp32-2432s028r/BOARD.md).

## Requirements

- ESP-IDF **6.1** (same as `esp32-invaders` on this machine: source
  `Projects/esp-idf/export.sh` before `make`)
- Classic ESP32 CYD on USB serial (default **`/dev/ttyUSB0`**)
- Host tests: `gcc` / `g++` (no IDF)

## Quick start

```bash
. /path/to/esp-idf/export.sh
cd esp32-chess
make test          # host: chess API + UI geom/FSM/calib math
make build         # firmware
make flash monitor # needs the board plugged in
```

Override port: `make flash PORT=/dev/ttyACM0`.

## Bring-up probes

| Target | Purpose |
|---|---|
| `make flash-esp32-lcdtest` | Orientation test card (ST7789) |
| `make flash-esp32-touchtest` | Finger-paint + UART raw ADC |
| `make flash-esp32-touchcalib` | Standalone 4-corner calib wizard → NVS |

After calib (standalone, in-game **CAL**, or first boot), touch mapping is stored
in NVS namespace `touch`. If NVS has no valid calib, the main app runs the wizard
automatically once before the board appears. Empty/corrupt load still falls back
to factory raw ranges (~200…3800) until the wizard succeeds.

## How to play

1. Flash the main app (`make flash`).
2. On first boot without saved calib, complete the 4-corner wizard; later use **CAL** if hits drift.
3. Tap a White piece, then a destination square.
4. Strip turns yellow while the engine thinks; then Black moves.

### Strip controls (right 80 px)

| Band | Action |
|---|---|
| Top | Side to move (yellow = thinking; red = mate; blue = stalemate) |
| NEW | New game |
| UNDO | Undo last human + engine pair |
| CAL | In-game 4-corner calibration |
| TIME | Cycle think time **1 s → 3 s → 5 s** (bar width) |

**Promotion:** strip becomes Q / R / B / N (color-coded). Tap one to finish the move.

Squares: engine index **a8 = 0 … h1 = 63**; board is left 240×240 (30 px cells).

## Host tests (TDD)

```bash
make test           # all
make test-chess     # chess_api
make test-ui        # geom, dirty mask, FSM, strip hits, touch calib math
```

No LCD/IDF in `host/` — pure `gcc` against `components/chess` and `components/ui`
(plus board calib math).

Engine search progress is **muted** by default (no UART spam during think). To
re-enable upstream-style depth lines, build with `-DCHESS_ENGINE_SERIAL`.

## Project layout

```text
components/board/   ST7789 + XPT2046 HAL, touch calib math/NVS/wizard
components/chess/   GPL engine + chess_api (in-tree port)
components/ui/      geom, dirty redraw, FSM, paint
host/chess|ui/      host unit tests
main/               game loop + lcd/touch/calib probe mains
```

Board paint is **square-level dirty** (`last_drawn[64]`), not a full framebuffer.

`chess_api` serializes engine access with an internal mutex. The UI task should
only call into it when the think worker is idle (except the worker’s own
`chess_think`); do not touch engine globals outside `chess_api`.

## License and attribution

This firmware **links a GPL-3.0 chess engine**. If you distribute binaries or a
modified tree, you must comply with **GPL-3.0** (provide corresponding source,
keep license notices).

| Piece | License / credit |
|---|---|
| Chess engine (`components/chess/`) | **GPL-3.0** — original by **Sergey Urusov** ([Hackster](https://www.hackster.io/Sergey_Urusov/esp32-chess-engine-c29dd9), GPL3+); packaged by [hpsaturn/esp32-chess-engine](https://github.com/hpsaturn/esp32-chess-engine). Full text: [`components/chess/LICENSE`](components/chess/LICENSE). |
| ESP-IDF / `esp_lcd` | Apache-2.0 (compatible with GPL-3 when combined) |
| This app’s UI/HAL glue | Same combined work — treat the distributed firmware as GPL-3.0-compatible |

Do not remove `components/chess/LICENSE` or the engine author credit in
`chess_engine.cpp` when publishing.
