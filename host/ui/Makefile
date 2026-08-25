CC ?= gcc
CFLAGS ?= -O0 -g -Wall -Wextra -Werror
BOARD_DIR := ../../components/board
UI_DIR := ../../components/ui
CHESS_DIR := ../../components/chess
CPPFLAGS := -I.. -I$(BOARD_DIR)/include -I$(UI_DIR)/include -I$(CHESS_DIR)/include

MATH_SRC := $(BOARD_DIR)/src/touch_calib_math.c
UI_PURE := $(UI_DIR)/src/chess_geom.c $(UI_DIR)/src/chess_dirty.c $(UI_DIR)/src/chess_fsm.c \
	$(UI_DIR)/src/chess_hints.c
UI_FONT := $(UI_DIR)/src/ui_font.c
UI_PIECES := $(UI_DIR)/src/ui_pieces.c

TEST_BINS := test_stub test_touch_calib test_chess_ui test_ui_font test_ui_pieces

.PHONY: all test clean

all: test

test_stub: test_stub.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ test_stub.c

test_touch_calib: test_touch_calib.c $(MATH_SRC)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ test_touch_calib.c $(MATH_SRC)

test_chess_ui: test_chess_ui.c $(UI_PURE)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ test_chess_ui.c $(UI_PURE)

test_ui_font: test_ui_font.c $(UI_FONT)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ test_ui_font.c $(UI_FONT)

test_ui_pieces: test_ui_pieces.c $(UI_PIECES)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ test_ui_pieces.c $(UI_PIECES)

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "== $$t =="; ./$$t || exit 1; done

clean:
	$(RM) $(TEST_BINS)
