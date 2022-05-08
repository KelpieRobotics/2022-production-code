from serialCommunication import arduinoCom
import serial.tools.list_ports
import logging

class unableToConnectToArduino(Exception):
    """The Arduino returned READY when another command was sent."""
    pass
class unassignedArduinoType(Exception):
    """The Arduino returned READY when another command was sent."""
    pass


class serialHandler:
    def __init__(self):
        logging.debug("Initializing Serial Handler")
        self.motorCom = None
        self.sensorCom = None

    def sendMotorCommands(self, command):
        """Sends commands to Motor Arduino
        Args:
            command: formmated string containing data necessary for movement
        Returns:
            Arduino's response
        """
        return self.motorCom.sendData(command)

    def sendSensorCommands(self, command):
        """Sends commands to Motor Arduino
        Args:
            command: formmated string containing data necessary to retrive sensor data
        Returns:
            Arduino's response
        """
        return self.sensorCom.sendData(command)

    def autoConnect(self):
        """Automatically establishes communication with onboard Arduinos
        Args:
            none
        Returns:
            none

        Raises:
            none
        """
        ports = serial.tools.list_ports.comports()
        logging.debug("Automatically establishing serial communication")
        logging.info(f"{len(ports)} available ports found")
        for port, desc, hwid in sorted(ports):
            # TODO : Verify which setting this is in based on device
            # if ("/dev/ttyUSB" in port) or ("/dev/ttyACM" in port): # For Raspbery Pi
            if ("COM" in port): # For testing on Windows Machines 
                self.assignPort(port)
        # Throws expections for not enough arduions are connected
        if (self.motorCom == None) and (self.sensorCom == None):
            raise unassignedArduinoType("There is no Arduino assigned to Motors and Sensors")
        if self.motorCom == None:
            raise unassignedArduinoType("There is no Arduino assigned to the Motor")
        if self.sensorCom == None:
            raise unassignedArduinoType("There is no Arduino assigned to the Sensor")
    def assignPort(self, port, baudRate=9600):
        """Assigns known devices to objects

        Args:
            port: The ComPort of the arduino
            baudRate: BaudRate of the arduino, default = 9600

        Returns:
            null

        Raises:
            null
        """
        # Assigns Unknown arduino to a temporary object and requests type
        temp = arduinoCom(port, baudRate)
        connectionSuccuss = temp.startConnection()

        if connectionSuccuss:
            typeData = temp.sendData("TYPE")
            # Assigns motor to motorCom Object
            if typeData == "MOTOR":
                logging.info(f"Motor Arduino at {port}")
                self.motorCom = temp
                temp = None
            # Assigns sensor to sensorCom Object
            elif typeData == "SENSOR":
                logging.info(f"Sensor Arduino at {port}")
                self.sensorCom = temp
                temp = None
            # Desregards other serial devices
            else:
                logging.info(f"Unknown Serial Device at {port}")
                temp.closeConnection()
                temp = None
        else:
            raise unableToConnectToArduino(f"There was an issue connecting to {port}")

    def closeAllConnections(self):
        """Closes Connections to Arduinos
        Args:
            none
        Returns:
            none
        """
        logging.debug("Closing all Serial Connections")
        if not(self.motorCom == None):
            self.motorCom.closeConnection()
        else:
            logging.debug("Cannot close Motor connection, as it was not established")
        if not(self.sensorCom == None):
            self.sensorCom.closeConnection()
        else:
            logging.debug("Cannot close Sensor connection, as it was not established")



if __name__ == '__main__':
    logging.basicConfig(filename = "shTestLog.log",encoding='utf-8', level=logging.DEBUG, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    sh = serialHandler()
    sh.autoConnect()
    sh.closeAllConnections()




# MIT License
#
#Copyright (c) 2017 Kevin Hughes
#Copyright (c) 2022 Sebastian Larrivee
#
#Permission is hereby granted, free of charge, to any person obtaining a copy
#of this software and associated documentation files (the "Software"), to deal
#in the Software without restriction, including without limitation the rights
#to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
#copies of the Software, and to permit persons to whom the Software is
#furnished to do so, subject to the following conditions:
#
#The above copyright notice and this permission notice shall be included in all
#copies or substantial portions of the Software.
#
#THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
#SOFTWARE.


from inputs import get_gamepad
import math
import threading

class XboxController(object):
    MAX_TRIG_VAL = math.pow(2, 8)
    MAX_JOY_VAL = math.pow(2, 15)

    def __init__(self):

        self.LeftJoystickY = 0
        self.LeftJoystickX = 0
        self.RightJoystickY = 0
        self.RightJoystickX = 0
        self.LeftTrigger = 0
        self.RightTrigger = 0
        self.LeftBumper = 0
        self.RightBumper = 0
        self.A = 0
        self.X = 0
        self.Y = 0
        self.B = 0
        self.LeftThumb = 0
        self.RightThumb = 0
        self.Back = 0
        self.Start = 0
        self.DPadX = 0
        self.DPadY = 0

        self._monitor_thread = threading.Thread(target=self._monitor_controller, args=())
        self._monitor_thread.daemon = True
        self._monitor_thread.start()


    def readMainButtons(self): # return the buttons/triggers that you care about in this methode
        a = self.A
        b = self.B
        x = self.X
        y = self.Y
        dx = self.DPadX
        dy = self.DPadY
        st = self.Start
        ba = self.Back
        return [a, b, x, y, st, ba, dx, dy]

    def readTriggers(self):
        lb = self.LeftBumper
        rb = self.RightBumper
        lt = self.LeftTrigger
        rt = self.RightTrigger
        return [lb, lt, rb, rt]

    def readAnalogSticks(self):
        lx = self.LeftJoystickX
        ly = self.LeftJoystickY
        rx = self.RightJoystickX
        ry = self.RightJoystickY
        lh = self.LeftThumb
        rh = self.RightThumb
        return [lh, lx, ly, rh, rx, ry]


    def _monitor_controller(self):
        while True:
            events = get_gamepad()
            for event in events:
                if event.code == 'ABS_Y':
                    self.LeftJoystickY = event.state / XboxController.MAX_JOY_VAL # normalize between -1 and 1
                elif event.code == 'ABS_X':
                    self.LeftJoystickX = event.state / XboxController.MAX_JOY_VAL # normalize between -1 and 1
                elif event.code == 'ABS_RY':
                    self.RightJoystickY = event.state / XboxController.MAX_JOY_VAL # normalize between -1 and 1
                elif event.code == 'ABS_RX':
                    self.RightJoystickX = event.state / XboxController.MAX_JOY_VAL # normalize between -1 and 1
                elif event.code == 'ABS_Z':
                    self.LeftTrigger = event.state / XboxController.MAX_TRIG_VAL # normalize between 0 and 1
                elif event.code == 'ABS_RZ':
                    self.RightTrigger = event.state / XboxController.MAX_TRIG_VAL # normalize between 0 and 1
                elif event.code == 'BTN_TL':
                    self.LeftBumper = event.state
                elif event.code == 'BTN_TR':
                    self.RightBumper = event.state
                elif event.code == 'BTN_SOUTH':
                    self.A = event.state
                elif event.code == 'BTN_NORTH':
                    self.X = event.state
                elif event.code == 'BTN_WEST':
                    self.Y = event.state
                elif event.code == 'BTN_EAST':
                    self.B = event.state
                elif event.code == 'BTN_THUMBL':
                    self.LeftThumb = event.state
                elif event.code == 'BTN_THUMBR':
                    self.RightThumb = event.state
                elif event.code == 'BTN_SELECT':
                    self.Back = event.state
                elif event.code == 'BTN_START':
                    self.Start = event.state
                elif event.code == 'ABS_HAT0X':
                    self.DPadX = event.state
                elif event.code == 'ABS_HAT0Y':
                    self.DPadY = event.state

if __name__ == '__main__':
    joy = XboxController()
    while True:
        print(f"{joy.readMainButtons()}{joy.readTriggers()}{joy.readAnalogSticks()}")
