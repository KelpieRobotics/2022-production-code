# Lists available serial com ports and device names


import serial.tools.list_ports
from serialCommunication import arduinoCom

ports = serial.tools.list_ports.comports()
print(ports)
out = ""
for port, desc, hwid in sorted(ports):
        print(port)
        if ("/dev/ttyUSB" in port) or ("/dev/ttyACM" in port): # For Raspbery Pi
                temp = arduinoCom(port, 9600)
                temp.startConnection()
                typeData = temp.sendData("TYPE")
                if typeData == "MOTOR":
                        print(f"Motor Arduino at {port}")
                        temp.closeConnection()
                        temp = None
                # Assigns sensor to sensorCom Object
                elif typeData == "SENSOR":
                        print(f"Sensor Arduino at {port}")
                        temp.closeConnection()
                # Desregards other serial devices
                else:
                        print(f"Unknown Serial Device at {port}, '{typeData}'")
                        temp.closeConnection()
                        temp = None              

print(out)
