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

from ControlSerial import ControlSerial


class Measurement():
    def __init__(self, array):
        self.pin = array[1]
        self.timestamp = array[2]
        self.count = array[3]
        self.sum = array[4]
        self.value = 0.0
        if self.count != 0:
            self.value = self.sum / self.count

        
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

        
    def add_analog_measure(self, params):
        """read an analog pin. See next function to understand the "secondary" parameter"""

        pin_symbol = params['pin'] 
        offset = params['offset']
        period = params['period']
        duration = params['duration']

        offset_s = int(offset // 1000)
        offset_ms = int(offset % 1000)
        period_s = int(period // 1000)
        period_ms = int(period % 1000)
        duration_s = int(duration // 1000)
        duration_ms = int(duration % 1000)

        if not isinstance(pin_symbol, str) or len(pin_symbol) <= 1 or pin_symbol[0] != 'A':
            raise ValueError('Expected "A"+number to specify the pin')
        
        s = pin_symbol[1:]
        if not s.isdigit():
            raise ValueError('Expected "A"+number to specify the pin')

        pin = int(s)
            
        self.arduino.send_command("a[%d,%d,%d,%d,%d,%d,%d]"
                                  % (pin, offset_s, offset_ms,
                                     duration_s, duration_ms,
                                     period_s, period_ms))

        

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

        
    def count_measurements(self):
        """count the number of available measurements"""
        result = self.arduino.send_command("n")
        return result[1]
        
        
    def get_measurement(self):
        """get an available measurements"""
        result = self.arduino.send_command("g")
        if result[1] >= 0:
            return Measurement(result)
        else:
            return None
        
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
    parser.add_argument('--port', default='COM3')
    args = parser.parse_args()
    
    ## ARDUINO connection
    port_arduino = args.port
    LEDs = ControlLight(port_arduino)
    #LEDs.arduino.set_debug(True)
    time.sleep(2.0)

    # blue LED
    blue_param = {'pin': 13,
            'offset': 500, #ms
            'period': 5*1000, #ms
            'duration': 2*1000, #ms
            'secondary': 1,
            'analog_value': 255,
            }

    # purple LED
    purple_param = {'pin': 11,
                'offset': 0,
                'period': 5*1000,
                'duration': 2*1000,
                'secondary': 0,
                'analog_value': 255,
                 }

    # analog measurements
    analog_param_1 = {'pin': 'A0',
                'offset': 0,
                'period': 50,
                'duration': 10
                 }

    analog_param_2 = {'pin': 'A1',
                'offset': 0,
                'period': 1000,
                'duration': 10
                 }

                 
    LEDs.add_digital_pulse(blue_param)
    LEDs.add_digital_pulse(purple_param)
    LEDs.set_secondary(purple_param, blue_param)
    #LEDs.add_analog_measure(analog_param_1)
    #LEDs.add_analog_measure(analog_param_2)
    LEDs.start_measurement(30*1000)

    #start = time.time()
    #while time.time() < start + 30.0:
    #    m = LEDs.get_measurement()
    #    if m: print(f't={m.timestamp}, pin={m.pin}, value={m.value}')
    #    time.sleep(0.02)

    LEDs.wait()
