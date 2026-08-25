#ifdef CHESS_HOST

#include "arduino_shim.h"

#include <chrono>
#include <thread>

SerialClass Serial;

unsigned long millis() {
    using clock = std::chrono::steady_clock;
    static const auto t0 = clock::now();
    return (unsigned long)std::chrono::duration_cast<std::chrono::milliseconds>(clock::now() - t0).count();
}

void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayMicroseconds(unsigned int us) {
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

void chess_engine_init(void) {
    xTaskCreate(taskOne, "TaskOne", 10000, nullptr, 1, nullptr);
}

#endif
