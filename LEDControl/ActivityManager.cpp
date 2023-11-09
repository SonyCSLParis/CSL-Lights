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

#include "Arduino.h"
#include "ActivityManager.h"

ActivityManager::ActivityManager() : number_activities_(0)
{
        for (uint8_t index = 0; index < kMaxActivities; index++) {
                activities_[index] = nullptr;
        }
}

ActivityManager::~ActivityManager()
{
        clear();
}

void ActivityManager::clear()
{
        for (uint8_t index = 0; index < number_activities_; index++) {
                delete activities_[index];
                activities_[index] = nullptr;
        }
        number_activities_ = 0;
}

bool ActivityManager::addActivity(IActivity *newactivity)
{
        bool retval = false;
        if (newactivity != nullptr
            && number_activities_ < kMaxActivities){
                activities_[number_activities_++] = newactivity;
                retval = true;
        }
        return retval;
}

IActivity **ActivityManager::getActivities()
{
        return activities_;
}

uint8_t ActivityManager::countActivities()
{
        return number_activities_;
}

uint8_t ActivityManager::availableSpace()
{
        return kMaxActivities - number_activities_;
}

IActivity *ActivityManager::getActivityOnPin(uint8_t pin)
{
        IActivity *activity = nullptr;
        for (uint8_t index = 0; index < number_activities_; index++) {
                if (activities_[index]->getPin() == pin) {
                        activity = activities_[index];
                        break;
                }
        }
        return activity;
}
