#ifndef __IActivity_h
#define __IActivity_h

#include <stdint.h>

class IActivity
{
public:
        virtual ~IActivity() = default;
        virtual uint8_t getPin() = 0;
        virtual void setPrimary(IActivity *activity) = 0;
        virtual bool isSecondary() = 0;
        virtual void start() = 0;
        virtual void update(int32_t ms) = 0;
        virtual void stop() = 0;
        virtual bool isOn() = 0;
        virtual void on() = 0;
        virtual void off() = 0;
};

#endif // __IActivity_h
