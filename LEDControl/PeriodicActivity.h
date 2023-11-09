/*
  
  Copyright (C) 2022 Sony Computer Science Laboratories
  
  Author(s) Peter Hanappe, Douglas Boari, Aliénor Lahlou
  
  free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see
  <http://www.gnu.org/licenses/>.
  
 */

#ifndef __PeriodicActivity_h
#define __PeriodicActivity_h

#include "BaseActivity.h"

class PeriodicActivity : public BaseActivity
{
public:
        
protected:
        int32_t offset_;
        int32_t period_;
        int32_t duration_;
        int32_t duration_on_;
        int32_t duration_off_;
        int32_t next_event_;
        bool should_be_on_;
        
public:

        PeriodicActivity(int8_t pin, int32_t start_offset_ms,
                         int32_t period, int32_t duration)
                : BaseActivity(pin),
                  offset_(start_offset_ms),
                  period_(period),
                  duration_(duration),
                  should_be_on_(false) {
                next_event_ = offset_;
                duration_on_ = duration;
                duration_off_ = period_ - duration;
        }
        
        virtual ~PeriodicActivity() = default;
        
        void start() override {
                next_event_ = offset_;
                if (duration_off_ == 0) {
                        on();
                        next_event_ = 0x7fffffff;
                } else if (duration_on_ == 0) {
                        off();
                        next_event_ = 0x7fffffff;
                }
        }
        
        void stop() override {
                off();
        }

        void update(int32_t ms) {
                if (ms >= next_event_) {
                        should_be_on_ = !should_be_on_;
                        if (should_be_on_) {
                                next_event_ += duration_on_;
                        } else {
                                next_event_ += duration_off_;
                        }
                }
                if (primary_ == nullptr) {
                        if (!on_ && should_be_on_)
                                on();
                        if (on_ && !should_be_on_)
                                off();
                } else {
                        if (!on_ && should_be_on_ && !primary_->isOn())
                                on();
                        else if (on_ && primary_->isOn())
                                off();
                        else if (on_ && !should_be_on_)
                                off();
                }
        }
};
        
#endif // __PeriodicActivity_h
