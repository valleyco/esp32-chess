# Cables & connectors — JST-SH 1.25 mm (CYD V3)

Buying guide for the **ESP32-2432S028R V3** expansion headers. The board uses
**JST-SH 1.25 mm** male ports (CN1, P1, P3, P2/P4). Pin functions are in
[`BOARD.md`](BOARD.md) and [`vendor-connector-manual.md`](vendor-connector-manual.md).

---

## Pitch warning

Many listings sell the wrong family:

| Family | Pitch | Use on this CYD? |
|---|---|---|
| **JST-SH** | **1.25 mm** | **Yes** |
| JST-PH | 2.0 mm | No |
| JST-XH | 2.54 mm | No |

Listings must say **1.25 mm** explicitly.

---

## CN1 / P3 — sensors & I2C (4-pin)

| Cable | What it is | Use | Search term |
|---|---|---|---|
| JST-SH 4-pin → premium male headers | JST on one end, Dupont males on the other | Breadboard / loose sensors (DHT11, BME280, ultrasonics, …) | `JST-SH 4 pin to male jumper wire 1.25mm` |
| JST-SH 4-pin female–female | JST both ends | Plug-and-play Adafruit **Stemma QT** / SparkFun **Qwiic** | `Qwiic cable 4-pin JST-SH 1.25mm` |

---

## P2 / P4 — audio / speaker (2-pin)

| Cable | What it is | Use | Search term |
|---|---|---|---|
| JST-SH 1.25 mm 2-pin with wires | 2-pin plug, loose red/black leads | Solder to ~**8 Ω, 1 W** speaker; plug into P2/P4 | `JST 1.25mm 2 pin connector with wire` |

---

## P1 — serial / VIN (4-pin)

| Cable | What it is | Use | Search term |
|---|---|---|---|
| JST-SH 4-pin → female Dupont | JST to separate female jumpers | External USB–UART adapter or 5 V supply on P1 | `JST-SH 4 pin to female jumper 1.25mm` |

---

## Shopping summary

| Board port | Pitch | Type | Pins | Example search / product |
|---|---|---|---|---|
| CN1 (I2C) | 1.25 mm | JST-SH | 4 | AliExpress: `JST SH 1.25 4-pin to Dupont` |
| P3 (GPIO) | 1.25 mm | JST-SH | 4 | [Adafruit Stemma QT / Qwiic breadboard cable](https://www.adafruit.com/product/4209) |
| P2/P4 (audio) | 1.25 mm | JST-SH | 2 | eBay: `JST 1.25mm 2-pin wire sets` |
| P1 (UART/VIN) | 1.25 mm | JST-SH | 4 | `JST-SH 4 pin to female jumper 1.25mm` |

---

## Buying tips

1. **Pitch first** — reject PH/XH unless the listing also shows 1.25 mm SH.
2. **Colors lie** — I2C cables are often Red=VCC, Black=GND, Blue=SDA, Yellow=SCL, but cheap looms swap colors. Match pins to [`BOARD.md`](BOARD.md), not wire color.
3. **Kit** — a **JST-SH 1.25 mm** housing/crimp kit (2/3/4/6 pin) is useful if you build more ESP32 gadgets.
