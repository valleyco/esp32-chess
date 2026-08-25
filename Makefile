IDF ?= idf.py
PORT ?= /dev/ttyUSB0
# Engine lives in git submodule components/chess (esp32-chess-lib).
CHESS_LIB ?= components/chess

# Default target is the plugged CYD (classic ESP32).
.PHONY: build build-esp32 build-esp32-lcdtest build-esp32-touchtest \
	build-esp32-touchcalib \
	flash flash-esp32 flash-esp32-lcdtest flash-esp32-touchtest \
	flash-esp32-touchcalib \
	monitor monitor-esp32 clean test test-chess test-ui bench

build: build-esp32

build-esp32:
	$(IDF) -B build-esp32 -D SDKCONFIG=sdkconfig.esp32 set-target esp32
	$(IDF) -B build-esp32 -D SDKCONFIG=sdkconfig.esp32 build

build-esp32-lcdtest:
	python3 host/lcdtest/make_panel_bmp.py
	$(IDF) -B build-esp32-lcdtest -D SDKCONFIG=sdkconfig.esp32 -D LCD_TEST=1 set-target esp32
	$(IDF) -B build-esp32-lcdtest -D SDKCONFIG=sdkconfig.esp32 -D LCD_TEST=1 build

build-esp32-touchtest:
	$(IDF) -B build-esp32-touchtest -D SDKCONFIG=sdkconfig.esp32 -D TOUCH_TEST=1 set-target esp32
	$(IDF) -B build-esp32-touchtest -D SDKCONFIG=sdkconfig.esp32 -D TOUCH_TEST=1 build

build-esp32-touchcalib:
	$(IDF) -B build-esp32-touchcalib -D SDKCONFIG=sdkconfig.esp32 -D TOUCH_CALIB=1 set-target esp32
	$(IDF) -B build-esp32-touchcalib -D SDKCONFIG=sdkconfig.esp32 -D TOUCH_CALIB=1 build

flash: flash-esp32

flash-esp32:
	$(IDF) -B build-esp32 -D SDKCONFIG=sdkconfig.esp32 -p $(PORT) flash

flash-esp32-lcdtest:
	$(IDF) -B build-esp32-lcdtest -D SDKCONFIG=sdkconfig.esp32 -D LCD_TEST=1 -p $(PORT) flash

flash-esp32-touchtest:
	$(IDF) -B build-esp32-touchtest -D SDKCONFIG=sdkconfig.esp32 -D TOUCH_TEST=1 -p $(PORT) flash

flash-esp32-touchcalib:
	$(IDF) -B build-esp32-touchcalib -D SDKCONFIG=sdkconfig.esp32 -D TOUCH_CALIB=1 -p $(PORT) flash

monitor: monitor-esp32

monitor-esp32:
	$(IDF) -B build-esp32 -D SDKCONFIG=sdkconfig.esp32 monitor

test: test-chess test-ui

test-chess:
	$(MAKE) -C $(CHESS_LIB) test

test-ui:
	$(MAKE) -C host/ui test

bench:
	$(MAKE) -C $(CHESS_LIB) bench

clean:
	rm -rf build-esp32 build-esp32-lcdtest build-esp32-touchtest \
		build-esp32-touchcalib \
		sdkconfig.esp32 sdkconfig sdkconfig.old \
		host/lcdtest/generated lcd_test_320x240.bmp
	$(MAKE) -C $(CHESS_LIB) clean
	$(MAKE) -C host/ui clean
