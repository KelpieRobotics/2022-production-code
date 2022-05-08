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




