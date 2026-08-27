# Contributing

Thanks for interest in **esp32-chess**. This firmware links a **GPL-3.0** engine;
contributions are accepted under the same license (see [`LICENSE`](LICENSE) and
[`CREDITS.md`](CREDITS.md)).

## Setup

```bash
git clone --recurse-submodules https://github.com/valleyco/esp32-chess.git
cd esp32-chess
# ESP-IDF 6.x on PATH (source export.sh), then:
make test          # host tests (no board required)
make build         # firmware (needs IDF)
```

Chess engine submodule: [`esp32-chess-lib`](https://github.com/valleyco/esp32-chess-lib)
at `components/chess`. Prefer opening PRs against the lib for engine/API changes,
and against this repo for UI / board HAL / app loop.

## Checks before a PR

- `make test` green (UI host + chess lib tests via submodule)
- For lib-only: `make -C components/chess test`
- No secrets, no `build-*` artifacts, no personal `sdkconfig.esp32`
- Keep Urusov / hpsaturn attribution intact

## Hardware notes

Primary bring-up target today is classic **ESP32** + SPI TFT (CYD-class boards).
Additional MCUs/panels (e.g. **ESP32-C3**, **ILI9341**) are welcome as board
ports under `components/board` / `docs/boards/` — please document pins and any
`swap_xy` / invert quirks in a short board note.

## Style

- Match nearby code; prefer small focused commits
- Host-test pure logic (geom, FSM, calib math, `chess_api`) before device-only paths
- Update [`PLAN.md`](PLAN.md) only when changing agreed product direction

## Issues

Bug reports: chip + panel, IDF version, `make test` result, and whether touch
calib was saved to NVS.
