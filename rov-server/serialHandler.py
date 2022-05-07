from serialCommunication import arduinoCom
import serial.tools.list_ports
import logging



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
            if "/dev/ttyUSB" or "/dev/ttyACM" in port:
                self.assignPort(port)
        # TODO : ERROR if there are not 2 devices available
                
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
        temp.startConnection()
        typeData = temp.sendData("TYPE")
        # Assigns motor to motorCom Object
        if typeData == "MOTOR":
            logging.info(f"Motor Arduino at {port})")
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

    def closePorts(self):
        """Closes Connections to Arduinos
        Args:
            none
        Returns:
            none
        """
        self.motorCom.closeConnection()
        self.sensorCom.closeConnection()
if __name__ == '__main__':
    sh = serialHandler()
    sh.autoConnect()




