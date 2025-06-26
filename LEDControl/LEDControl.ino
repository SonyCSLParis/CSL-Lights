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
#include <ArduinoSerial.h>
#include <RomiSerial.h>
#include <RomiSerialErrors.h>
#include <stdint.h>
#include "DigitalPulse.h"
#include "AnalogMeasure.h"
#include "ActivityManager.h"
#include "BoardFactory.h"
#include "ITimer.h"
#include "IBoard.h"
#include "EndSwitch.h"
#include "Relay.h"

using namespace romiserial;

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_add_digital_pulse(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg);
void handle_set_secondary(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_add_analog_measure(IRomiSerial *romiSerial, int16_t *args,
                                 const char *string_arg);
void handle_stop_measurements(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg);
void handle_start_measurements(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg);
void handle_count_measurements(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg);
void handle_get_measurement(IRomiSerial *romiSerial, int16_t *args,
                            const char *string_arg);
void handle_add_relay(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_is_active(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_reset(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'd', 8, false, handle_add_digital_pulse },
        { 'r', 1, false, handle_add_relay },
        { 'a', 7, false, handle_add_analog_measure },
        { 's', 2, false, handle_set_secondary },
        { 'b', 2, false, handle_start_measurements },
        { 'e', 0, false, handle_stop_measurements },
        { 'n', 0, false, handle_count_measurements },
        { 'g', 0, false, handle_get_measurement },
        { 'R', 0, false, handle_reset },
        { 'A', 0, false, handle_is_active },
        { '?', 0, false, send_info },
};

static int kMaxActivitiesCode = 1;
static int kBadStartCode = 2;
static int kBadPeriodCode = 3;
static int kBadDurationCode = 4;
static int kBadPinCode = 5;

static char *kMaxActivitiesMessage = "Too many activities";
static char *kBadStartMessage = "Bad start";
static char *kBadPeriodMessage = "Bad period";
static char *kBadDurationMessage = "Bad duration";
static char *kBadPinMessage = "Bad pin";

#if defined(ARDUINO_SAM_DUE)
ArduinoSerial serial(SerialUSB);
#else
ArduinoSerial serial(Serial);
#endif

RomiSerial romiSerial(serial, serial, handlers, sizeof(handlers) / sizeof(MessageHandler));
ActivityManager activityManager;

extern volatile int32_t timer_interrupts_;

CircularBuffer measurement_buffer;


void setup()
{
#if defined(ARDUINO_SAM_DUE)
        SerialUSB.begin(0);
        while(!SerialUSB)
                ;
#else
        Serial.begin(115200);
        while (!Serial)
                ;
#endif


        IBoard& board = BoardFactory::get();
        ITimer& timer = board.get_timer();
        timer.init();

        timer.start(activityManager.getActivities(),
                    activityManager.countActivities());
}

void loop()
{
        romiSerial.handle_input();
        delay(10);
}

void send_info(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        romiSerial->send("LED Control v0.1");
        romiSerial->send("Commands:");
        romiSerial->send("create pulse: #d[13,0,0,0,500,1,0,255]");
        romiSerial->send("pin, delay (s),delay (ms), time high (s), time high (ms), period (s), period (ms), val (0-255)");
        romiSerial->send("start pulse: #b[1000, 0]");
        romiSerial->send("duration (s), duration (ms)");
        romiSerial->send("#e - stop, #R - reset, #A - status");
}

void handle_add_digital_pulse(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg)
{
        int8_t pin = (uint8_t) args[0];
        int32_t start_delay = (int32_t) args[1] * 1000 + args[2];
        int32_t duration = (int32_t) args[3] * 1000 + args[4];
        int32_t period = (int32_t) args[5] * 1000 + args[6];
        int value = args[7];
    
        if (start_delay < 0)
                romiSerial->send_error(kBadStartCode, kBadStartMessage);
        else if (period <= 0)
                romiSerial->send_error(kBadPeriodCode, kBadPeriodMessage);
        else if (duration < 0 || duration > period)
                romiSerial->send_error(kBadDurationCode, kBadDurationMessage);
        else if (activityManager.availableSpace() == 0) 
                romiSerial->send_error(kMaxActivitiesCode, kMaxActivitiesMessage);
        else {
                auto activity = new DigitalPulse(pin, start_delay, period,
                                                 duration, value);
                activityManager.addActivity(activity);
                romiSerial->send_ok();
        }
}

void handle_set_secondary(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        int8_t primary_pin = (uint8_t) args[0];
        int8_t secondary_pin = (uint8_t) args[1];

        IActivity *primary_activity = activityManager.getActivityOnPin(primary_pin);
        IActivity *secondary_activity = activityManager.getActivityOnPin(secondary_pin);

        if (primary_activity == nullptr
            || secondary_activity == nullptr) {
                romiSerial->send_error(kBadPinCode, kBadPinMessage);                
        } else {
                secondary_activity->setPrimary(primary_activity);
                romiSerial->send_ok();
        }
}

void handle_add_analog_measure(IRomiSerial *romiSerial, int16_t *args,
                               const char *string_arg)
{        
        int8_t pin = (uint8_t) args[0];
        int8_t analog_pin = number_to_analog_pin(pin);
        int32_t start_delay = (int32_t) args[1] * 1000 + args[2];
        int32_t duration = (int32_t) args[3] * 1000 + args[4];
        int32_t period = (int32_t) args[5] * 1000 + args[6];
        
        if (analog_pin == -1)
                romiSerial->send_error(kBadPinCode, kBadPinMessage);
        else if (start_delay < 0)
                romiSerial->send_error(kBadStartCode, kBadStartMessage);
        else if (period <= 0)
                romiSerial->send_error(kBadPeriodCode, kBadPeriodMessage);
        else if (duration < 0 || duration > period)
                romiSerial->send_error(kBadDurationCode, kBadDurationMessage);
        else if (activityManager.availableSpace() == 0) 
                romiSerial->send_error(kMaxActivitiesCode, kMaxActivitiesMessage);
        else {
                auto activity = new AnalogMeasure(measurement_buffer, analog_pin, pin, start_delay, period, duration);
                activityManager.addActivity(activity);
                romiSerial->send_ok();
        }
}

int8_t number_to_analog_pin(int8_t number)
{
        int8_t analog_pin;
        switch (number) {
        default:
                analog_pin = -1;
                break;
        case 0:
                analog_pin = A0;
                break;
#if defined(ESP32)
        case 1:
                analog_pin = A3;  // GPIO39
                break;
        case 2:
                analog_pin = A6;  // GPIO34
                break;
        case 3:
                analog_pin = A7;  // GPIO35
                break;
        case 4:
                analog_pin = A4;  // GPIO32
                break;
        case 5:
                analog_pin = A5;  // GPIO33
                break;
        case 6:
                analog_pin = A18; // GPIO25
                break;
        case 7:
                analog_pin = A19; // GPIO26
                break;
#else
        case 1:
                analog_pin = A1;
                break;
        case 2:
                analog_pin = A2;
                break;
        case 3:
                analog_pin = A3;
                break;
        case 4:
                analog_pin = A4;
                break;
        case 5:
                analog_pin = A5;
                break;
#if defined(A6)
        case 6:
                analog_pin = A6;
                break;
#endif
#if defined(A7)
        case 7:
                analog_pin = A7;
                break;
#endif
#endif
#if defined(A8)
        case 8:
                analog_pin = A8;
                break;
#endif
#if defined(A9)
        case 9:
                analog_pin = A9;
                break;
#endif
#if defined(A10)
        case 10:
                analog_pin = A10;
                break;
#endif
#if defined(A11)
        case 11:
                analog_pin = A11;
                break;
#endif
        }
        
        return analog_pin;
}

void handle_start_measurements(IRomiSerial *romiSerial, int16_t *args,
                               const char *string_arg)
{
        int32_t duration = (int32_t) args[0] * 1000 + args[1];
        if (duration > 0) {
                auto endSwitch = new EndSwitch(duration);
                activityManager.addActivity(endSwitch);
        }
        
        IBoard& board = BoardFactory::get();
        ITimer& timer = board.get_timer();
        timer.start(activityManager.getActivities(),
                    activityManager.countActivities());
        
        romiSerial->send_ok();
}

void handle_stop_measurements(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg)
{
        IBoard& board = BoardFactory::get();
        ITimer& timer = board.get_timer();
        timer.stop();
        
        romiSerial->send_ok();
}

void handle_add_relay(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        uint8_t pin = args[0];
        auto relay = new Relay(pin);
        activityManager.addActivity(relay);
        romiSerial->send_ok();
}

void handle_is_active(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        IBoard& board = BoardFactory::get();
        ITimer& timer = board.get_timer();
        if (timer.isActive()) {
                romiSerial->send("[0,1]"); 
        } else {
                romiSerial->send("[0,0]"); 
        }
}

void handle_reset(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        IBoard& board = BoardFactory::get();
        ITimer& timer = board.get_timer();
        timer.stop();
        activityManager.clear();
        romiSerial->send_ok();
}

void handle_count_measurements(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        int8_t n = measurement_buffer.available();
        String r = String("[0,") + n + String("]");
        romiSerial->send(r.c_str()); 
}

void handle_get_measurement(IRomiSerial *romiSerial, int16_t *args, const char *string_arg)
{
        if (measurement_buffer.available() > 0) {
                AnalogMeasurement m;
                measurement_buffer.get(m);
                String r = (String("[0,")
                            + m.pin_ + String(",")
                            + m.timestamp_ + String(",")
                            + m.count_ + String(",")
                            + m.sum_ + String("]"));
                romiSerial->send(r.c_str()); 
                
        } else {
                romiSerial->send("[0,-1,0,0,0]"); 
        }
}