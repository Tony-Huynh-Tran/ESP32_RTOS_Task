# ESP32Task

A lightweight Arduino library for ESP32 to create FreeRTOS tasks and thread-safe shared variables — in just one line each.

> Made by Tony Huynh Tran — June 2026

---

## Requirements

- **Hardware:** ESP32 (any variant)
- **Arduino IDE:** 1.8.0 or later
- **Dependency:** FreeRTOS — included in the ESP32 Arduino core, no extra install needed

---

## Installation

**Option 1 — Arduino IDE (recommended)**

1. Download this repo as a ZIP file
2. In Arduino IDE: **Sketch → Include Library → Add .ZIP Library...**
3. Select the downloaded ZIP → Done ✅

**Option 2 — Manual**

1. Download and extract the repo
2. Copy the folder into `Documents/Arduino/libraries/ESP32Task/`
3. Restart Arduino IDE

---

## Quick Start

```cpp
#include <ESP32Task.h>

SHARED(float, temperature);   // declare like a normal variable
SHARED(bool,  led_state);

void read_sensor() {
    temperature = 27.5;        // write like a normal variable
}

void blink_led() {
    led_state = !(bool)led_state;
    digitalWrite(2, (bool)led_state);
}

void setup() {
    Serial.begin(115200);
    pinMode(2, OUTPUT);

    Task_begin();  // required — call before any task()

    task(read_sensor, 1, 1000);   // Core 1, every 1000ms
    task(blink_led,   0,  500);   // Core 0, every 500ms
}

void loop() {
    vTaskDelay(portMAX_DELAY);  // loop() does nothing
}
```

---

## API Reference

### Setup

| Call | Description |
|------|-------------|
| `Task_begin()` | Initialize the library — call once at the top of `setup()` |

### Creating Tasks

```cpp
task(fn, core, ms);
```

| Parameter | Type | Description |
|-----------|------|-------------|
| `fn` | `void (*)()` | Function to run (no arguments, no return) |
| `core` | `int` | Core to pin the task to: `0` or `1` |
| `ms` | `int` | Delay between each run in milliseconds |

> Advanced: `task(fn, core, ms, stack, priority)` — stack defaults to `4096` bytes, priority defaults to `1`.

### Shared Variables

```cpp
SHARED(type, name)           // default-initialized
SHARED(type, name, value)    // with initial value
```

Use exactly like a normal variable — no locks, no extra calls:

```cpp
SHARED(float, temperature, 25.0);

temperature = 30.1;          // write from any core
float t = temperature;       // read from any core
```

Each `SHARED` variable has its own mutex. Reads and writes are automatically protected — no deadlocks possible.

### Supported Types

Any plain type works: `float`, `int`, `bool`, `double`, `uint8_t`, etc.

> Structs and classes work only if they are trivially copyable.

---

## How It Works

- `task()` calls `xTaskCreatePinnedToCore()` under the hood
- Each `SHARED<T>` holds a value and a `SemaphoreHandle_t` mutex
- `operator=` (write) and `operator T()` (read) lock/unlock automatically
- Copy constructor is deleted — each variable owns its own mutex, no shared-mutex bugs

---

## Example Sketch

The included `BasicExample.ino` runs 4 tasks across 2 cores:

```
[Core1] Temp: 28.4 | Humid: 61.2 | Counter: 5
[Core1] Temp: 31.0 | Humid: 58.7 | Counter: 6
```

Use it to verify everything is running before adding your own logic.

---

## Troubleshooting

**Tasks not starting**

- Make sure `Task_begin()` is called before any `task()` call
- Check that `loop()` ends with `vTaskDelay(portMAX_DELAY)` — not `delay()` or an empty body

**Stack overflow crash**

- Increase the `stack` parameter: `task(fn, core, ms, 8192)`
- Default 4096 bytes is enough for most tasks; LVGL or heavy logic may need more

**Data looks corrupted between cores**

- Make sure the variable is declared with `SHARED()`, not as a plain global
- Plain globals have no protection across cores

**Compilation error: "Most Vexing Parse"**

- Use `SHARED(float, x)` not `Shared<float> x()` — the macro handles this automatically

---

## License

MIT License — Free to use in personal and commercial projects.
