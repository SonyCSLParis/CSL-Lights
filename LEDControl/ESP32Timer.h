#ifndef __ESP32Timer_h
#define __ESP32Timer_h

#include "ITimer.h"

class ESP32Timer : public ITimer
{
public:
    ESP32Timer();
    ~ESP32Timer() override = default;

    void init() override;
    void start(IActivity **a, int n) override;
    void stop() override;
    bool isActive() override;
};

#endif // __ESP32Timer_h