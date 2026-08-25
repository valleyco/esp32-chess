# ESP32 Chess on CYD

Playable chess on the ESP32-2432S028R (CYD): ST7789 + resistive touch, engine
ported from [hpsaturn/esp32-chess-engine](https://github.com/hpsaturn/esp32-chess-engine)
(Sergey Urusov / GPL-3.0).

## Status

Step 6 done: UI geom/FSM/dirty (host-tested) + dirty board paint.
Host: `make test`. Firmware paints starting position then dirty e2e4 (flash when board connected).

See `PLAN.md`.

## License

Firmware that vendors the engine is GPL-3.0-compatible; keep engine LICENSE and
attribution when distributing.
