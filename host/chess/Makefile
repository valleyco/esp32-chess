CC ?= gcc
CXX ?= g++
CFLAGS ?= -O0 -g -Wall -Wextra -Werror
CXXFLAGS ?= $(CFLAGS) -std=c++17
ENGINE_CXXFLAGS := $(CXXFLAGS) -Wno-parentheses -Wno-sign-compare -Wno-unused-parameter \
	-Wno-unused-variable -Wno-type-limits -Wno-dangling-else -Wno-return-type -Wno-format
CHESS_DIR := ../../components/chess
CPPFLAGS := -I.. -I$(CHESS_DIR)/include -DCHESS_HOST -DCHESS_ENGINE_LIBRARY

TEST_BINS := test_chess_api
BENCH_BIN := bench_chess

.PHONY: all test bench clean

all: test

chess_api.o: $(CHESS_DIR)/src/chess_api.cpp $(CHESS_DIR)/include/chess_api.h \
		$(CHESS_DIR)/include/chess_engine_internal.h $(CHESS_DIR)/include/arduino_shim.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

chess_engine.o: $(CHESS_DIR)/src/chess_engine.cpp $(CHESS_DIR)/include/chess_engine_internal.h \
		$(CHESS_DIR)/include/arduino_shim.h
	$(CXX) $(ENGINE_CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

arduino_shim_host.o: $(CHESS_DIR)/src/arduino_shim_host.cpp $(CHESS_DIR)/include/arduino_shim.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test_chess_api: test_chess_api.c chess_api.o chess_engine.o arduino_shim_host.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ test_chess_api.c chess_api.o chess_engine.o arduino_shim_host.o -pthread

bench_chess: bench_chess.c chess_api.o chess_engine.o arduino_shim_host.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ bench_chess.c chess_api.o chess_engine.o arduino_shim_host.o -pthread

test: $(TEST_BINS) $(BENCH_BIN)
	@for t in $(TEST_BINS); do echo "== $$t =="; ./$$t || exit 1; done
	@echo "== bench_chess =="
	./$(BENCH_BIN)

bench: $(BENCH_BIN)
	./$(BENCH_BIN) --print-only

clean:
	$(RM) $(TEST_BINS) $(BENCH_BIN) chess_api.o chess_engine.o arduino_shim_host.o
