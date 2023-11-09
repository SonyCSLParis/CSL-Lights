#ifndef __IBoard_h
#define __IBoard_h

#include "ITimer.h"

class IBoard
{
public:
        virtual ~IBoard() = default;
        virtual ITimer& get_timer() = 0;
};
        
#endif // __IBoard_h

