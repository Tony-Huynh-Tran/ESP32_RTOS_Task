#pragma once
#include <Arduino.h>

// ═══════════════════════════════════════════════════════════
//  ESP32_RTOS_Task  v1.1.0
//  Task + biến dùng chung tự động cho ESP32
// ═══════════════════════════════════════════════════════════
//
//  CÁCH DÙNG NHANH:
//
//    #include <ESP32_RTOS_Task.h>
//
//    SHARED(float, temperature);   // khai báo như biến thường
//
//    void read_sensor() {
//        temperature = 27.5;       // ghi như biến thường
//    }
//
//    void update_display() {
//        float t = temperature;    // đọc như biến thường
//    }
//
//    void setup() {
//        Task_begin();
//        task(read_sensor,    1, 1000);  // Core 1, mỗi 1000ms
//        task(update_display, 0,   10);  // Core 0, mỗi 10ms
//    }
//
//    void loop() { vTaskDelay(portMAX_DELAY); }
//
// ═══════════════════════════════════════════════════════════

// ─── Task_begin() ─────────────────────────────────────────
// Bắt buộc gọi 1 lần đầu tiên trong setup()
inline void Task_begin() {
    // Không cần làm gì thêm — mutex được tạo trong từng Shared<T>
}

// ─── task() ───────────────────────────────────────────────
// fn       : hàm muốn chạy (void, không tham số)
// core     : 0 hoặc 1
// ms       : delay sau mỗi lần chạy (mặc định 0)
// stack    : bộ nhớ cấp cho task  (mặc định 4096 bytes)
// priority : độ ưu tiên           (mặc định 1)

struct _TaskParams {
    void (*fn)();
    int ms;
};

inline void task(void (*fn)(), int core, int ms = 0,
                 int stack = 4096, int priority = 1)
{
    _TaskParams* p = new _TaskParams{fn, ms};
    xTaskCreatePinnedToCore(
        [](void* param) {
            _TaskParams* p = (_TaskParams*)param;
            for (;;) {
                p->fn();
                if (p->ms > 0) vTaskDelay(pdMS_TO_TICKS(p->ms));
            }
        },
        "", stack, (void*)p, priority, NULL, core
    );
}

// ─── Shared<T> ────────────────────────────────────────────
// Mỗi biến có mutex riêng — không dùng chung, không deadlock
// Dùng như biến thường: ghi/đọc trực tiếp, không cần gọi gì thêm

template<typename T>
class Shared {
    T                    _val;
    mutable SemaphoreHandle_t _mutex;

public:
    Shared(T init = T{}) : _val(init) {
        _mutex = xSemaphoreCreateMutex();
    }

    // Không cho copy — mỗi Shared có mutex riêng, copy sẽ sai
    Shared(const Shared&)            = delete;
    Shared& operator=(const Shared&) = delete;

    // Ghi:  temperature = 27.5
    Shared& operator=(T val) {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        _val = val;
        xSemaphoreGive(_mutex);
        return *this;
    }

    // Đọc:  float t = temperature
    operator T() const {
        xSemaphoreTake(_mutex, portMAX_DELAY);
        T val = _val;
        xSemaphoreGive(_mutex);
        return val;
    }
};

// ─── SHARED macro ─────────────────────────────────────────
// SHARED(float, temperature)        →  Shared<float> temperature
// SHARED(float, temperature, 25.0)  →  Shared<float> temperature(25.0)
//
// Dùng _SHARED_INIT để tránh Most Vexing Parse:
//   SHARED(float, x)    → Shared<float> x        ← object, không phải function
//   SHARED(float, x, 0) → Shared<float> x{0}
#define _SHARED_HAS_INIT(a, b, c, ...) c
#define _SHARED_INIT_1(type, name)        Shared<type> name
#define _SHARED_INIT_2(type, name, init)  Shared<type> name{init}
#define SHARED(type, name, ...) \
    _SHARED_HAS_INIT(0, ##__VA_ARGS__, _SHARED_INIT_2, _SHARED_INIT_1)(type, name, ##__VA_ARGS__)
