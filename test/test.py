from serial import Serial
from CSLlight import ControlLight
import time

#arduino_port = "COM5"
arduino_port = "/dev/ttyACM0"

ms = 1
sec = 1000
min = 60*sec

blue_param = {'pin': 2,
              'offset': 0,
              'period': 1*sec,
              'duration': 0.5*sec,
              'secondary': 0,
              'analog_value': 255,
              }

arduino_light = ControlLight(arduino_port)
arduino_light.add_digital_pulse(blue_param)
arduino_light.start_measurement()
time.sleep(20)
arduino_light.stop_measurement()
