#if defined(ARDUINO_ARCH_ESP32)

#include "ESP32.h"

ESP32Board::ESP32Board()
    : timer_()
{
}

ITimer& ESP32Board::get_timer()
{
    return timer_;
}

#endif // if ESP32