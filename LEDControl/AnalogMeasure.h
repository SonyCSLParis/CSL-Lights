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

#ifndef __AnalogMeasure_h
#define __AnalogMeasure_h

#include "PeriodicActivity.h"

struct AnalogMeasurement
{
public:
        int8_t pin_;
        int32_t timestamp_;
        int32_t count_;
        uint32_t sum_;
        
        AnalogMeasurement()
                : pin_(-1),
                  timestamp_(0),
                  count_(0),
                  sum_(0) {
        }
        
        AnalogMeasurement(int8_t pin, int32_t timestamp, int32_t count, uint32_t sum)
                : pin_(pin),
                  timestamp_(timestamp),
                  count_(count),
                  sum_(sum) {
        }
};


class CircularBuffer
{
protected:
#if defined(ARDUINO_AVR_UNO)
        static const int length_ = 16;
#elif defined(ARDUINO_SAM_DUE)
        static const int length_ = 64;
#elif defined(ESP32)
        static const int length_ = 128;
#else
        static const int length_ = 32;  // Default fallback
#endif
        
        int readpos_;
        int writepos_;
        AnalogMeasurement data_[length_];
        
public:
        
        CircularBuffer()
                : readpos_(0),
                  writepos_(0) {
        }

        int8_t available() {
                int8_t available;
        
                lock();
                if (writepos_ >= readpos_)
                        available = writepos_ - readpos_;
                else 
                        available = length_ + writepos_ - readpos_;
                unlock();

                return available;
        }

        int8_t space() {
                int8_t space;
        
                lock();
                if (writepos_ >= readpos_)
                        space = length_ + readpos_ - writepos_ - 1;
                else
                        space = readpos_ - writepos_ - 1;
                unlock();
        
                return space;
        }

        void put(AnalogMeasurement& m) {
                lock();
                data_[writepos_++] = m;                
                if (writepos_ >= length_) {
                        writepos_ = 0;
                }
                unlock();
        }

        void get(AnalogMeasurement& m) {
                lock();
                m = data_[readpos_++];
                if (readpos_ >= length_) {
                        readpos_ = 0;
                }
                unlock();
        }

protected:
        void lock() {
        }

        void unlock() {
        }
};

class AnalogMeasure : public PeriodicActivity
{
public:
        CircularBuffer& buffer_;
        AnalogMeasurement m_;
        int32_t timestamp_;
        int32_t count_;
        uint32_t sum_;
        int8_t pin_user_;
        
        AnalogMeasure(CircularBuffer& buffer, int8_t pin, int8_t pin_user, int32_t start_offset_ms, int32_t period, int32_t duration)
                : PeriodicActivity(pin, start_offset_ms, period, duration),
                  buffer_(buffer),
                  m_(),
                  timestamp_(0),
                  count_(0),
                  sum_(0),
                  pin_user_(pin_user) {
        }

        void update(int32_t ms) {
                PeriodicActivity::update(ms);
                if (isOn()) {
                        measure(ms);
                }
        }
        
        void on() override {
                on_ = true;
                timestamp_ = 0;
                count_ = 0;
                sum_ = 0;
        }
        
        void off() override {
                on_ = false;
                m_.pin_ = pin_user_;
                m_.timestamp_ = timestamp_;
                m_.count_ = count_;
                m_.sum_ = sum_;
                buffer_.put(m_);
        }

        void measure(int32_t ms) {
                if (timestamp_ == 0) {
                        timestamp_ = millis();
                        //timestamp_ = ms;
                }
                count_++;
                sum_ += analogRead(pin_);
        }
};

#endif // __AnalogMeasure_h
