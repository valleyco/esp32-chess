#ifndef CHESS_HOST

#include "arduino_shim.h"

SerialClass Serial;

void chess_engine_init(void) {
    xTaskCreate(taskOne, "TaskOne", 10000, nullptr, 1, nullptr);
}

#endif
