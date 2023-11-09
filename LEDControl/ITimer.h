#ifndef __ITimer_h
#define __ITimer_h

#include "IActivity.h"

class ITimer
{
public:
        virtual ~ITimer() = default;

        virtual void init() = 0;
        virtual void start(IActivity **a, int n) = 0;
        virtual void stop() = 0;
        virtual bool isActive() = 0;
};
        
#endif // __ITimer_h

