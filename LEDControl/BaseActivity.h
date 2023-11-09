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

#ifndef __BaseActivity_h
#define __BaseActivity_h

#include "IActivity.h"

class BaseActivity : public IActivity
{
public:
        
protected:
        int8_t pin_;
        bool on_;
        IActivity *primary_;
        
public:

        BaseActivity(int8_t pin)
                : pin_(pin),
                  on_(false),
                  primary_(nullptr) {
        }
        
        ~BaseActivity() = default;

        uint8_t getPin() override {
                return pin_;
        }

        bool isOn() override {
                return on_;
        }

        bool isSecondary() override {
                return primary_ != nullptr;
        }

        void setPrimary(IActivity *activity) override {
                primary_ = activity;
        }
};
        
#endif // __BaseActivity_h
