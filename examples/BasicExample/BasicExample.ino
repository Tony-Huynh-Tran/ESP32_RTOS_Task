// ═══════════════════════════════════════════════════════════
//  ESP32_RTOS_Task — Ví dụ cơ bản
//  4 task, 2 core, biến dùng chung tự động
// ═══════════════════════════════════════════════════════════

#include <ESP32_RTOS_Task.h>

// ─── Khai báo biến dùng chung ─────────────────────────────
SHARED(float, temperature);       // = 0.0 mặc định
SHARED(float, humidity);
SHARED(bool,  led_state);
SHARED(int,   counter, 0);        // khởi tạo = 0

// ─── Core 1 — Task 1: đọc cảm biến mỗi 1 giây ────────────
void read_sensor() {
    temperature = random(200, 350) / 10.0;
    humidity    = random(400, 800) / 10.0;
    counter     = (int)counter + 1;
}

// ─── Core 1 — Task 2: in log mỗi 2 giây ──────────────────
void serial_log() {
    Serial.printf("[Core1] Temp: %.1f | Humid: %.1f | Counter: %d\n",
        (float)temperature,
        (float)humidity,
        (int)counter);
}

// ─── Core 0 — Task 1: cập nhật màn hình mỗi 10ms ─────────
void update_display() {
    // Ví dụ với LVGL:
    // lv_label_set_text(ui_Label, String((float)temperature).c_str());
    // display.update();
}

// ─── Core 0 — Task 2: nhấp nháy LED mỗi 500ms ────────────
void blink_led() {
    led_state = !(bool)led_state;
    digitalWrite(2, (bool)led_state);
}

// ─────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);

    Task_begin();  // ← bắt buộc gọi trước task()

    //         hàm            core  delay(ms)
    task(read_sensor,    1,   1000);
    task(serial_log,     1,   2000);
    task(update_display, 0,     10);
    task(blink_led,      0,    500);
}

void loop() {
    vTaskDelay(portMAX_DELAY);  // loop() không làm gì
}
