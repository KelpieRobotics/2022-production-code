import serial
import time
ser = serial.Serial("COM4", 9600, timeout=1)
print(ser.name)         # check which port was really used

while True:
    
    try:
        if(ser == None):
            ser = serial.Serial("COM4", 9600, timeout=1)
            print("reconnecting")
        print(ser.is_open)
        sendString = "TYPE"
        ser.write(sendString.encode('utf-8'))
        serialFeedback = ser.readline().decode('utf-8').rstrip()
        print(serialFeedback)
    except:
        if(not(ser == None)):
            ser.close()
            ser = None
            print("disconnecting")
    print(ser)
    time.sleep(1)