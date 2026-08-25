# ESP32 Chess on CYD

Playable chess on the ESP32-2432S028R (CYD): ST7789 + resistive touch, engine
ported from [hpsaturn/esp32-chess-engine](https://github.com/hpsaturn/esp32-chess-engine)
(Sergey Urusov / GPL-3.0).

## Status

Step 8 done: strip NEW / UNDO / CAL / TIME, promotion picker, mate/stalemate.
Host: `make test`. Flash when connected: `make flash && make monitor`.

See `PLAN.md`.

### Strip controls (right 80 px)

| Band | Action |
|---|---|
| Top | Side to move (yellow = thinking; red/blue = mate/stale) |
| NEW | New game |
| UNDO | Undo last human+engine pair |
| CAL | In-game 4-corner calibration |
| TIME | Cycle think time 1s → 3s → 5s (bar width) |

Promotion: strip becomes Q / R / B / N (color-coded).

## License

Firmware that vendors the engine is GPL-3.0-compatible; keep engine LICENSE and
attribution when distributing.
