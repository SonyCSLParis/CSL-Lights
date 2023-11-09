#ifndef __EndSwitch_h
#define __EndSwitch_h

#include "IActivity.h"
#include "BoardFactory.h"
#include "ITimer.h"
#include "IBoard.h"

class EndSwitch : public IActivity
{
protected:
        int32_t end_;
        bool done_;
        
public:
        EndSwitch(int32_t end)
                : end_(end),
                  done_(false) {
        }
        
        ~EndSwitch() override = default;

        uint8_t getPin() override {
                return 0xff;
        }
        
        void start() override {
                done_ = false;
        }
                
        void update(int32_t ms) override {
                if (ms >= end_) {
                        IBoard& board = BoardFactory::get();
                        ITimer& timer = board.get_timer();
                        timer.stop();
                }
        }
                
        void stop() override {
                end_ = 0x7ffffff;
                done_ = true;
        }
        
        void on() override {
        }
        
        void off() override {
        }
        
        bool isOn() override {
                return !done_;
        }
        
        bool isDone() {
                return done_;
        }
        
        bool isSecondary() {
                return false;
        }
        
        void setPrimary(IActivity *activity) {
                // pass
        }
};

#endif // __EndSwitch_h
