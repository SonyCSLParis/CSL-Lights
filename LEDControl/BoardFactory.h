#ifndef __BoardFactory_h
#define __BoardFactory_h

#include "IBoard.h"

class BoardFactory
{
public:
        static IBoard& get();
};

#endif // __Board_h

