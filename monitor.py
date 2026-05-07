import serial
import sys

port = "COM8"
baud = 115200

print(f"Monitoring {port} @ {baud} baud... (press Ctrl+C to stop)\n")

ser = serial.Serial(port, baud, timeout=1)
try:
    while True:
        line = ser.readline()
        if line:
            sys.stdout.write(line.decode("utf-8", errors="replace"))
            sys.stdout.flush()
except KeyboardInterrupt:
    print("\nStopped.")
finally:
    ser.close()
