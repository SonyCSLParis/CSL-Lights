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

#ifndef __DigitalPulse_h
#define __DigitalPulse_h

#include "PeriodicActivity.h"

class DigitalPulse : public PeriodicActivity
{
public:
        int value_;
        
        DigitalPulse(int8_t pin, int32_t start_offset, int32_t period,
                     int32_t duration, int analog_value)
                : PeriodicActivity(pin, start_offset, period, duration),
                  value_(analog_value) {
                pinMode(pin_, OUTPUT);                
        }

        void on() override {
                analogWrite(pin_, value_);
                on_ = true;
        }
        
        void off() override {
                analogWrite(pin_, 0);
                on_ = false;
        }
};

#endif // __DigitalPulse_h
