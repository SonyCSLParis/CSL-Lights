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
void handle_stop_measurements(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg);
void handle_start_mesurements(IRomiSerial *romiSerial, int16_t *args,
                              const char *string_arg);
void handle_add_relay(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_is_active(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);
void handle_reset(IRomiSerial *romiSerial, int16_t *args, const char *string_arg);

const static MessageHandler handlers[] = {
        { 'd', 8, false, handle_add_digital_pulse },
        { 'r', 1, false, handle_add_relay },
        { 's', 2, false, handle_set_secondary },
        { 'b', 2, false, handle_start_mesurements },
        { 'e', 0, false, handle_stop_measurements },
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
        romiSerial->send("[\"LightControl\",\"0.1\"]"); 
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

void handle_start_mesurements(IRomiSerial *romiSerial, int16_t *args,
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
