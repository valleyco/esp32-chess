#pragma once

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

typedef bool boolean;

#ifndef F
#define F(x) x
#endif

#ifndef B1111
#define B1111 0x0F
#endif

class String {
public:
    String() = default;
    String(const char *s) : data_(s ? s : "") {}
    String(const std::string &s) : data_(s) {}
    String(int v) : data_(std::to_string(v)) {}
    String(long v) : data_(std::to_string(v)) {}
    String(unsigned long v) : data_(std::to_string(v)) {}
    String(char c) : data_(1, c) {}
    String(double v, int decimals) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.*f", decimals, v);
        data_ = buf;
    }

    unsigned int length() const { return (unsigned int)data_.size(); }

    char charAt(unsigned int i) const {
        return i < data_.size() ? data_[i] : '\0';
    }

    char operator[](unsigned int i) const { return charAt(i); }

    void trim() {
        const auto start = data_.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            data_.clear();
            return;
        }
        const auto end = data_.find_last_not_of(" \t\r\n");
        data_ = data_.substr(start, end - start + 1);
    }

    int indexOf(char ch) const {
        const auto pos = data_.find(ch);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    int indexOf(char ch, unsigned int from) const {
        if (from >= data_.size()) {
            return -1;
        }
        const auto pos = data_.find(ch, from);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    int indexOf(const String &s) const {
        const auto pos = data_.find(s.data_);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    int indexOf(const char *s, unsigned int from) const {
        if (!s || from >= data_.size()) {
            return -1;
        }
        const auto pos = data_.find(s, from);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    String substring(unsigned int start) const {
        if (start >= data_.size()) {
            return String("");
        }
        return String(data_.substr(start));
    }

    String substring(unsigned int start, unsigned int end) const {
        if (start >= data_.size() || start >= end) {
            return String("");
        }
        const unsigned int len = end > data_.size() ? (unsigned int)data_.size() - start : end - start;
        return String(data_.substr(start, len));
    }

    int toInt() const { return std::atoi(data_.c_str()); }

    void toUpperCase() {
        for (char &c : data_) {
            c = (char)std::toupper((unsigned char)c);
        }
    }

    String &operator+=(const String &rhs) {
        data_ += rhs.data_;
        return *this;
    }

    String &operator+=(const char *rhs) {
        if (rhs) {
            data_ += rhs;
        }
        return *this;
    }

    String &operator+=(char c) {
        data_ += c;
        return *this;
    }

    const char *c_str() const { return data_.c_str(); }

    bool operator==(const char *rhs) const { return data_ == (rhs ? rhs : ""); }
    bool operator==(const String &rhs) const { return data_ == rhs.data_; }
    bool operator!=(const char *rhs) const { return !(*this == rhs); }
    bool operator!=(const String &rhs) const { return !(*this == rhs); }

private:
    std::string data_;
};

inline String operator+(const String &lhs, const String &rhs) {
    String out = lhs;
    out += rhs;
    return out;
}

inline String operator+(const String &lhs, const char *rhs) {
    String out = lhs;
    out += rhs;
    return out;
}

inline String operator+(const String &lhs, char rhs) {
    String out = lhs;
    out += rhs;
    return out;
}

inline String operator+(char lhs, const String &rhs) {
    String out;
    out += lhs;
    out += rhs;
    return out;
}

/* Engine Serial is muted by default (UART cost during think).
 * Define CHESS_ENGINE_SERIAL to dump search progress to stderr again. */
class SerialClass {
public:
    void begin(unsigned long) {}

#ifdef CHESS_ENGINE_SERIAL
    void print(char c) { std::fputc(c, stderr); }
    void print(const char *s) { if (s) std::fputs(s, stderr); }
    void print(int v) { std::fprintf(stderr, "%d", v); }
    void print(long v) { std::fprintf(stderr, "%ld", v); }
    void print(unsigned long v) { std::fprintf(stderr, "%lu", v); }
    void print(const String &s) { print(s.c_str()); }

    void println() { std::fputc('\n', stderr); }
    void println(const char *s) {
        print(s);
        println();
    }
    void println(const String &s) {
        print(s);
        println();
    }
    void println(int v) {
        print(v);
        println();
    }
#else
    void print(char) {}
    void print(const char *) {}
    void print(int) {}
    void print(long) {}
    void print(unsigned long) {}
    void print(const String &) {}

    void println() {}
    void println(const char *) {}
    void println(const String &) {}
    void println(int) {}
#endif

    int available() { return 0; }

    int read() { return 255; }

    String readString() { return String(""); }
};

extern SerialClass Serial;

void taskOne(void *parameter);

#ifdef CHESS_HOST

#include <atomic>
#include <cstdint>
#include <thread>

unsigned long millis();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

using TaskFunction_t = void (*)(void *);
using TaskHandle_t = void *;
using BaseType_t = int;
using UBaseType_t = unsigned int;

inline BaseType_t xTaskCreate(TaskFunction_t fn, const char *, uint32_t, void *param,
                              UBaseType_t, TaskHandle_t *) {
    static std::atomic<bool> started{false};
    if (!started.exchange(true)) {
        std::thread([fn, param]() { fn(param); }).detach();
    }
    return 1;
}

#else

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

inline unsigned long millis() {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

inline void delay(unsigned long ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

inline void delayMicroseconds(unsigned int us) {
    if (us >= 1000) {
        vTaskDelay(pdMS_TO_TICKS((us + 999) / 1000));
    } else {
        esp_rom_delay_us(us);
    }
}

#endif

void chess_engine_init(void);
