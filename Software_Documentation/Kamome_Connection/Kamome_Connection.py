import serial
import time

ser = serial.Serial("COM3", 9600, timeout=2)

# Arduino resets when serial opens
time.sleep(2)

print("connected")

# Read startup message
print(ser.readline().decode())

# Send ping
ser.write(b"PING\n")

# Read responses
while True:
    line = ser.readline()

    if line:
        print("RAW:", repr(line))
        print("TEXT:", line.decode().strip())
    else:
        print("timeout")
        break