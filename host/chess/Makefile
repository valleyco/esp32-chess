CC ?= gcc
CXX ?= g++
CFLAGS ?= -O0 -g -Wall -Wextra -Werror
CXXFLAGS ?= $(CFLAGS) -std=c++17
ENGINE_CXXFLAGS := $(CXXFLAGS) -Wno-parentheses -Wno-sign-compare -Wno-unused-parameter \
	-Wno-unused-variable -Wno-type-limits -Wno-dangling-else -Wno-return-type -Wno-format
CHESS_DIR := ../../components/chess
CPPFLAGS := -I.. -I$(CHESS_DIR)/include -DCHESS_HOST -DCHESS_ENGINE_LIBRARY

TEST_BINS := test_chess_api

.PHONY: all test clean

all: test

chess_api.o: $(CHESS_DIR)/src/chess_api.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

chess_engine.o: $(CHESS_DIR)/src/chess_engine.cpp
	$(CXX) $(ENGINE_CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

arduino_shim_host.o: $(CHESS_DIR)/src/arduino_shim_host.cpp
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c -o $@ $<

test_chess_api: test_chess_api.c chess_api.o chess_engine.o arduino_shim_host.o
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $@ test_chess_api.c chess_api.o chess_engine.o arduino_shim_host.o -pthread

test: $(TEST_BINS)
	@for t in $(TEST_BINS); do echo "== $$t =="; ./$$t || exit 1; done

clean:
	$(RM) $(TEST_BINS) chess_api.o chess_engine.o arduino_shim_host.o
