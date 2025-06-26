#if defined(ARDUINO_ARCH_ESP32)

#include "ESP32Timer.h"
#include "esp32-hal-timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"

hw_timer_t *timer = nullptr;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile int32_t current_time_ms_ = 0;
IActivity **activities_ = nullptr;
int num_activities_ = 0;
volatile bool running_ = false;

ESP32Timer::ESP32Timer() {
    // Constructor implementation
}

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    if (running_) {
        for (int i = 0; i < num_activities_; i++) {
            if (!activities_[i]->isSecondary())
                activities_[i]->update(current_time_ms_);
        }
        for (int i = 0; i < num_activities_; i++) {
            if (activities_[i]->isSecondary())
                activities_[i]->update(current_time_ms_);
        }
    }
    current_time_ms_++;
    portEXIT_CRITICAL_ISR(&timerMux);
}

void ESP32Timer::init() {
    // Create timer with 1MHz frequency (1 tick = 1µs)
    timer = timerBegin(1000000);
    timerAttachInterrupt(timer, &onTimer);
    // Set alarm to trigger every 1000µs (1ms)
    timerAlarm(timer, 1000, true, 0);
}

void ESP32Timer::start(IActivity **a, int n) {
    stop();
    activities_ = a;
    num_activities_ = n;
    for (int i = 0; i < n; i++) activities_[i]->start();
    current_time_ms_ = 0;
    running_ = true;
    timerStart(timer);
}

void ESP32Timer::stop() {
    running_ = false;
    if (timer) {
        timerStop(timer);
    }
    for (int i = 0; i < num_activities_; i++) {
        if (activities_[i]) activities_[i]->stop();
    }
    num_activities_ = 0;
    activities_ = nullptr;
}

bool ESP32Timer::isActive() {
    return running_;
}

#endif // if ESP32
