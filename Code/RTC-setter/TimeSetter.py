import serial
import time, datetime, os

serialPort = serial.Serial(port=serialPortNo, baudrate=921600, bytesize=8, timeout=10, stopbits=serial.STOPBITS_ONE)

now = datetime.now()
#example: 50,37,20,17,11,23

dt_string = now.strftime("%S,%M,%H,%d,%m,%y")
print(dt_string)

serialPort.write(dt_string) 