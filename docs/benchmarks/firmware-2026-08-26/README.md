# Firmware snapshot for step-19 device bench (deferred)

Built from app `805860c` + submodule `f9f675b` (nps UART logging + 1s boot probe).

**Binaries are gitignored** (`*.bin`). They live only on this machine after a local build:

| File | Role |
|---|---|
| `esp32-chess.bin` | App @ 0x10000 |
| `bootloader.bin` | Bootloader @ 0x1000 |
| `partition-table.bin` | Partition table @ 0x8000 |

## Flash later (when CYD is plugged in)

```bash
. /path/to/esp-idf/export.sh
cd esp32-chess
# Prefer rebuild from the same SHAs if bins are gone:
git checkout 805860c   # or current master if still equivalent
git submodule update
make build flash monitor
```

Or flash these saved images (from this directory, classic ESP32, adjust PORT):

```bash
esptool.py --chip esp32 -p /dev/ttyUSB0 -b 460800 --before default-reset --after hard-reset \
  write-flash --flash-mode dio --flash-freq 40m --flash-size 4MB \
  0x1000 bootloader.bin 0x8000 partition-table.bin 0x10000 esp32-chess.bin
```

## What to capture into the baseline doc

Look for UART lines:

- `device_bench 1s start: ... nps` (automatic at boot)
- `engine done depth=... nodes=... (... nps)` after in-game thinks (TIME 1/3/5 s)
