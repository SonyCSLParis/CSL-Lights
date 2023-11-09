#include "BoardFactory.h"

#if defined(ARDUINO_AVR_UNO)
#include "Uno.h"
Uno this_board_;

#elif defined(ARDUINO_SAM_DUE)
#include "Due.h"
Due this_board_;

#else
#error Unknown board
#endif
        
IBoard& BoardFactory::get()
{
        return this_board_;
}
