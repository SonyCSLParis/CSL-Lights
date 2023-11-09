#include "Uno.h"

Uno::Uno()
        : timer_()
{
}

ITimer& Uno::get_timer()
{
        return timer_;
}
