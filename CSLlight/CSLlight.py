"""
  
  Copyright (C) 2022 Sony Computer Science Laboratories
  
  Author(s) Peter Hanappe, Aliénor Lahlou
  
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
  
"""

import time
import json
import argparse

from CSLserial import ControlSerial

class ControlLight():
    def __init__(self, arduino_port):
        self.arduino_port = arduino_port
        self.arduino = ControlSerial(self.arduino_port)


    def add_digital_pulse(self, params):
        """create a digital pulse function. See next function to understand the "secondary" parameter"""

        pin = params['pin'] 
        offset = params['offset']
        period = params['period']
        duration = params['duration']
        #secondary = params['secondary'] #secondary=0: indépendant, secondary=1: dependant
        analog_value = params['analog_value']

        offset_s = int(offset // 1000)
        offset_ms = int(offset % 1000)
        period_s = int(period // 1000)
        period_ms = int(period % 1000)
        duration_s = int(duration // 1000)
        duration_ms = int(duration % 1000)
        
        self.arduino.send_command("d[%d,%d,%d,%d,%d,%d,%d,%d]"
                                  % (pin, offset_s, offset_ms,
                                     duration_s, duration_ms,
                                     period_s, period_ms,
                                     analog_value))

    def set_secondary(self, primary, secondary): 
        """Set a pulse as a secondary to another pulse. """
        primary_pin = primary['pin'] 
        secondary_pin = secondary['pin'] 
        self.arduino.send_command(f"s[{primary_pin},{secondary_pin}]")


    def start_measurement(self, duration=0):
        """start the experiment"""
        sec = duration // 1000
        ms = duration % 1000
        self.arduino.send_command(f"b[{sec},{ms}]")

        
    def stop_measurement(self):
        """stop the experiment"""
        self.arduino.send_command("e")
        #self.arduino.reset_arduino()

        
    def is_active(self):
        """stop the experiment"""
        result = self.arduino.send_command("A")
        if result[1] == 0:
            return False
        else:
            return True

        
    def wait(self):
        """wait until the experiment is over"""
        active = True
        while active:
            active = self.is_active()
            time.sleep(0.5)

        
    def reset(self):
        """Stop the measurements and erase the current set-up"""
        self.arduino.send_command("R")


if __name__ == "__main__":


################ PARAMETERS

    parser = argparse.ArgumentParser(prog = 'SwitchLEDs')
    parser.add_argument('--port', default='COM5')
    args = parser.parse_args()
    
    ## ARDUINO connection
    port_arduino = args.port
    LEDs = ControlLight(port_arduino)
    LEDs.arduino.set_debug(True)
    time.sleep(2.0)

    blue_param = {'pin': 3,
            'offset': 500, #ms
            'period': 5*1000, #ms
            'duration': 2*1000, #ms
            'secondary': 1,
            'analog_value': 255,
            }

    # purple
    purple_param = {'pin': 11,
                'offset': 0,
                'period': 5*1000,
                'duration': 2*1000,
                'secondary': 0,
                'analog_value': 255,
                 }

                 
    LEDs.add_digital_pulse(blue_param)
    LEDs.add_digital_pulse(purple_param)
    LEDs.set_secondary(purple_param, blue_param)
    LEDs.start_measurement(30*s)
