#ifndef __Relay_h
#define __Relay_h

#include "BaseActivity.h"

class Relay : public BaseActivity
{
        
public:
        Relay(int8_t pin) : BaseActivity(pin) {
                pinMode(pin_, OUTPUT);                
        }
        
        ~Relay() override = default;
        
        void start() override {
                on();
        }
                
        void update(int32_t ms) override {
        }
                
        void stop() override {
                off();
        }
        
        void on() override {
                if (pin_ > 0)
                        digitalWrite(pin_, HIGH);
        }
        
        void off() override {
                if (pin_ > 0)
                        digitalWrite(pin_, LOW);
        }
};

#endif // __Relay_h
