# ESP32-2432S028R (CYD, Version 3 USB-C)

Project board manual for the **Cheap Yellow Display** used as the primary
target. Copied from `esp32-invaders` and kept in sync for this chess project’s
verified ST7789 + XPT2046 unit. Values under **Verified config** are what
**this unit** needed after bring-up — other CYD revisions can differ
(especially the LCD controller).

| | |
|---|---|
| Status in repo | **Primary** — board HAL ported; lcdtest/touchtest available |
| Host serial | typically `/dev/ttyUSB0` (CH340) |
| Make defaults | `make build` / `flash` / `monitor` → classic ESP32 |
| HAL | `components/board/src/cyd_display.c`, `cyd_touch.c` (no Invaders input zones) |

---

## Identity

| | |
|---|---|
| Common name | **CYD** — Cheap Yellow Display |
| Model | **ESP32-2432S028R** (Version 3, dual **USB-C**) |
| MCU | Classic **ESP32-WROOM** (not S3/C3) |
| Display | 2.8″ TFT, **320×240** landscape in firmware (“2432” ≈ 240×320 portrait) |
| Touch | Resistive **XPT2046** |
| USB | Dual USB-C: data+power (CH340) and power-only |
| Expansion | **JST-SH 1.25 mm** headers (CN1, P1, P3, P2/P4) |

Older CYD photos/docs often show dual Micro-USB; **V3 uses USB-C**. Pin
functions below match the V3 connector chart in
[`vendor-connector-manual.md`](vendor-connector-manual.md).

---

## Connectors

All expansion connectors are **JST-SH 1.25 mm** male. Cable buying guide:
[`connectors.md`](connectors.md).

```text
          [USB-C data/power]   [USB-C power]
  +---------------------------------------------------+
  |  [P1: Serial]   [P2/P4: Audio]    [Micro-SD]      |
  |                    ESP32-WROOM                    |
  |  [CN1: I2C]     [P3: GPIO Extended]               |
  +---------------------------------------------------+
```

### CN1 — I2C / sensors (4-pin)

| Pin | Signal | Notes |
|---|---|---|
| 1 | 3V3 | Power out for peripherals |
| 2 | GPIO27 | GPIO / OneWire-friendly |
| 3 | GPIO22 | I2C SCL on V3 |
| 4 | GND | |

### P3 — Extended I/O (4-pin)

| Pin | Signal | Notes |
|---|---|---|
| 1 | GPIO21 | **Shared with LCD backlight** — do not use freely |
| 2 | GPIO22 | Same net as CN1 pin 3 |
| 3 | GPIO35 | **Input-only**, no internal pull-up |
| 4 | GND | No VCC on this header |

### P1 — UART / alternate power (4-pin)

| Pin | Signal | Notes |
|---|---|---|
| 1 | GND | |
| 2 | RX (GPIO3) | UART0 RX |
| 3 | TX (GPIO1) | UART0 TX |
| 4 | VIN / VCC | **5 V** into onboard regulator |

### P2 / P4 — Speaker (2-pin)

| | |
|---|---|
| Pins | Differential amp output (onboard **SC8002B**) |
| Drive | External speaker ~**8 Ω, 1 W** |
| Control | Amp input from **GPIO26** (PWM / DAC) |

### Dual USB-C

| Port | Role |
|---|---|
| Main (USB-CN1) | 5 V + serial data via **CH340** (flash / monitor) |
| Secondary | **Power only** — no data |
| Warning | Do **not** feed both ports from separate supplies at once |

### Micro-SD (TF)

SPI (often called VSPI on classic ESP32 docs):

| Signal | GPIO |
|---|---|
| MOSI | 23 |
| MISO | 19 |
| SCK | 18 |
| CS | 5 |

---

## On-board I/O we use

### Display — ST7789 on SPI2

Many listings say **ILI9341**. **This unit is ST7789.** Wrong driver →
stripe / hail wrap.

| Signal | GPIO | Notes |
|---|---|---|
| MOSI | 13 | |
| SCLK | 14 | |
| CS | 15 | |
| DC | 2 | |
| RST | — | Not wired (`-1`); software reset |
| Backlight | 21 | Active high; also on P3 pin 1 |

### Touch — XPT2046 on SPI3

Separate bus from the LCD (do not share SPI2 with the panel).

| Signal | GPIO | Notes |
|---|---|---|
| SCLK | 25 | |
| MOSI | 32 | |
| MISO | 39 | Input-only |
| CS | 33 | |
| IRQ | 36 | Input-only; **no internal pulls** |

### Audio (not wired in firmware yet)

GPIO26 → SC8002B → P2/P4 speaker header (Step 10 / D5 still open).

---

## Verified config (this unit)

### Panel

| Setting | Value |
|---|---|
| Driver | `esp_lcd` **ST7789** |
| Orientation | Landscape **320×240** |
| Transform | `swap_xy` on, `mirror_x` on, `mirror_y` off |
| Color | **RGB**, invert **off** |
| Clock | ~**20 MHz** |
| Pixels | RGB565 with **byte-swap on TX** |

### Touch map

- ADC → panel **320×240** landscape
- **No** axis swap, **no** mirror on the touch path
- Raw ~200…3800; Z1 pressure threshold; IRQ active-low when usable
- **Never** enable pull-up/down on GPIO36/39

### Invaders on glass

Native Midway **256×224** RGB565 at **(32, 8)** on 320×240 (borders for HUD).

Touch zones → `emu_handle_keyboard` (P1):

```text
┌─────────────┬─────────────┐
│   CREDIT    │  P1 START   │  top ~40 px
├──────┬──────┴──────┬──────┤
│ LEFT │             │ RIGHT│  side ~70 px
│      │   (play)    │      │
├──────┴─────────────┴──────┤
│         P1 SHOT           │  bottom ~56 px
└───────────────────────────┘
```

---

## Buses at a glance

```text
ESP32-WROOM
 ├─ SPI2 ── ST7789 LCD     (13/14/15/2, BL 21)
 ├─ SPI3 ── XPT2046 touch  (25/32/39/33, IRQ 36)
 ├─ VSPI ── Micro-SD       (23/19/18/5)   [unused by game yet]
 ├─ GPIO26 ─ SC8002B amp → P2/P4         [unused by game yet]
 └─ USB-C ─ CH340 ─ UART0                flash / monitor
```

---

## Project usage

| Make target | Purpose |
|---|---|
| `make build` / `flash` / `monitor` | Game (display + touch) |
| `make flash-esp32-lcdtest` | Full-panel reference BMP probe |
| `make flash-esp32-touchtest` | Finger-paint touch probe |

Also buildable without this board’s wiring: `build-s3` / `flash-s3`, `build-c3` / `flash-c3`.

Code:

- Display: `components/board/src/cyd_display.c`
- Touch: `components/board/src/cyd_touch.c`
- Zones: `components/board/src/cyd_input.c`

---

## Free GPIOs / risks

| Resource | Risk |
|---|---|
| GPIO21 | Backlight — shared with P3 |
| GPIO22 | On both CN1 and P3 |
| GPIO35 | Input-only, no internal pull |
| GPIO26 | Audio amp |
| SD SPI pins | Free only if SD unused |
| GPIO36 / 39 | Touch MISO / IRQ — input-only, no pulls |

Best expansion path for sensors: **CN1** (3V3 + GPIO27 + GPIO22 as SCL). Add
an external SDA on a free GPIO if you bring up I2C (confirm V3 wiring — CN1
documents SCL on 22; SDA may need another pin depending on board silk/rev).

---

## Gotchas

1. Listing says ILI9341 ≠ silicon — confirm with a known 320×240 blit.
2. Fix geometry before color (invert / RGB↔BGR last).
3. GPIO36/39: no internal pulls; pull config aborts bring-up.
4. LCD and touch use **different** SPI hosts on this pinout.
5. Do not dual-power the two USB-C ports from separate supplies.
6. P3 pin 1 (GPIO21) will blink/blank the panel if used as GPIO.
7. Other CYD SKUs may need different pins or invert-on — re-verify per unit.

---

## Sources

- Cable / connector buying guide:
  [`connectors.md`](connectors.md)
- Vendor / connector chart (adapted into this repo):
  [`vendor-connector-manual.md`](vendor-connector-manual.md)
- Living plan: [`PLAN.md`](../../../PLAN.md) (D3 display, D4 input)
- Board index: [`../README.md`](../README.md)
