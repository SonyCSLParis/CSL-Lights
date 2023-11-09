#ifndef __Uno_h
#define __Uno_h

#include "IBoard.h"
#include "UnoTimer.h"

class Uno : public IBoard
{
protected:
        UnoTimer timer_;
public:
        Uno();
        ~Uno() override = default;
        ITimer& get_timer() override;
};
        
#endif // __Uno_h

