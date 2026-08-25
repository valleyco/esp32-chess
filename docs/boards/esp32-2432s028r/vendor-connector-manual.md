# ESP32-2432S028R (Version 3 USB-C) — connector reference

Vendor-style connector / pinout notes for the CYD V3 USB-C board. Kept as a
source companion to [`BOARD.md`](BOARD.md) (project bring-up + verified
display/touch live there).

All expansion connectors use standard **JST-SH 1.25 mm** male headers.

---

## Visual layout

```text
          [USB-C (Data/Power)]   [USB-C (Power)]
  +---------------------------------------------------+
  |  [P1: Serial]   [P2/P4: Audio]    [Micro-SD Slot] |
  |                    ESP32-WROOM                    |
  |  [CN1: I2C]     [P3: GPIO Extended]               |
  +---------------------------------------------------+
```

---

## Connector pinouts

### CN1 — I2C & sensors (4-pin JST)

Primary expansion for I2C / OneWire-style peripherals.

| Pin | Signal | Notes |
|---|---|---|
| 1 | V3.3 | 3.3 V out for external components |
| 2 | GPIO27 | GPIO (good for OneWire data) |
| 3 | GPIO22 | I2C SCL on Version 3 |
| 4 | GND | Ground |

### P3 — Extended I/O (4-pin JST)

Auxiliary pins; several overlap onboard hardware.

| Pin | Signal | Notes |
|---|---|---|
| 1 | GPIO21 | **Shared with display backlight** — toggling flickers/blanks the panel |
| 2 | GPIO22 | Same line as CN1 pin 3 |
| 3 | GPIO35 | **Input-only**; no internal pull-up; good for buttons / digital inputs |
| 4 | GND | Ground only (no positive rail on this header) |

### P1 — Serial programming & power (4-pin JST)

External UART, flashing, or 5 V power without USB.

| Pin | Signal | Notes |
|---|---|---|
| 1 | GND | Ground |
| 2 | RX (GPIO3) | UART0 receive |
| 3 | TX (GPIO1) | UART0 transmit |
| 4 | VCC / VIN | 5 V into the onboard regulator |

### P2 / P4 — Speaker / audio (2-pin JST)

Next to the onboard **SC8002B** amplifier.

| | |
|---|---|
| Pins 1 & 2 | Differential audio out; drives an external speaker (~8 Ω, 1 W) |
| Control | Amp driven from **GPIO26** (PWM or DAC) |

### Dual USB Type-C

Version 3 replaces the older dual Micro-USB layout.

| Port | Role |
|---|---|
| Main (USB-CN1) | 5 V + serial via onboard **CH340** (programming / monitor) |
| Secondary | Power only — no data lines |
| Warning | Do not power both ports from separate supplies at the same time |

### Micro-SD (TF)

Push-push slot on SPI (classic ESP32 “VSPI” lines):

| Signal | GPIO |
|---|---|
| MOSI | 23 |
| MISO | 19 |
| SCK | 18 |
| CS | 5 |
