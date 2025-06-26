#ifndef __ESP32Board_h
#define __ESP32Board_h

#include "IBoard.h"
#include "ESP32Timer.h"

class ESP32Board : public IBoard
{
protected:
    ESP32Timer timer_;
public:
    ESP32Board();
    ~ESP32Board() override = default;
    ITimer& get_timer() override;
};

#endif // __ESP32Board_h