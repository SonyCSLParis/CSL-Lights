#ifndef __UnoTimer_h
#define __UnoTimer_h

#include "ITimer.h"

class UnoTimer : public ITimer
{
public:
        UnoTimer();
        ~UnoTimer() override = default;

        void init() override;
        void start(IActivity **a, int n) override;
        void stop() override;
        bool isActive() override;
};
        
#endif // __UnoTimer_h

