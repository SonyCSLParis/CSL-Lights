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


//
// Created by douglas on 12/02/2021.
//

#ifndef __ActivityManager_h
#define __ActivityManager_h

#include "IActivity.h"


class ActivityManager
{
private:
        static const uint8_t kMaxActivities = 10;
        
        IActivity *activities_[kMaxActivities];
        uint8_t number_activities_;

public:
        explicit ActivityManager();
        virtual ~ActivityManager();
        
        bool addActivity(IActivity *activity);
        IActivity **getActivities();
        uint8_t countActivities();
        uint8_t availableSpace();
        IActivity *getActivityOnPin(uint8_t pin);
        void clear();
};

#endif //__ActivityManager_h
