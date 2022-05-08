from cmath import log
import serial
import logging
import time



class arduinoCom:
    def __init__(self, comPort: str, baudRate: int) -> None:
        """Comunication Protocol for Arduino <-> Raspberry Pi

        Args:
            comPort: Communication port path for Specificed Arduino
            baudRate: Baudrate for SerialPort 

        Returns:
            None

        Raises:
            None
        """
        self.comPort = comPort
        self.baudRate = baudRate
        logging.debug(f"New arduino {comPort} at {baudRate}")


    def startConnection(self):
        """Establishes Connection with the Arduino through Serial

        Args:
            None

        Returns:
            None

        Raises:
            None
        """
        try:
            self.serialConnection = serial.Serial(self.comPort, self.baudRate, timeout=1)
            self.serialConnection.flush()
            logging.info(f"Connected to {self.comPort} at {self.baudRate}")
            return True
        except serial.SerialException as e:
            logging.error(f"Device not available to {self.comPort} at {self.baudRate} Exception: {e}")
            return False



        # serialFeedback = self.serialConnection.readline().decode('utf-8').rstrip()
        # if(serialFeedback == "READY"):
        #     print(f"connected to {self.comPort}")
        #     logging.debug(f"Arduino at {")
        # TODO : Needs not ready error

        

    def sendData(self, data: str) -> str:
        """Sends and recives data from the Arduino

        Args:
            data: The data that is being send to the Arduino

        Returns:
            The Arduinos's response

        Raises:
            None
        """
        # Verifies if there is an active connection
        if(self.serialConnection == None):
            logging.warning(f"Attempting reconnection on {self.comPort}")
            self.startConnection()

        # Send data to arduino
        try:
            sendString = data if data.endswith("\n") else data + "\n"
            self.serialConnection.write(sendString.encode('utf-8'))
            serialFeedback = self.serialConnection.readline().decode('utf-8').rstrip()
            return serialFeedback

        # Exception handling for Serial Communication Issue
        except serial.SerialTimeoutException as e:
            logging.error(f"{self.comPort} - SerialTimeoutException - Exception: {e}")
            if(not(self.serialConnection == None)):
                self.closeConnection()
            return "ERROR"
        except serial.SerialException as e:
            logging.error(f"{self.comPort} - SerialException - Exception: {e}")
            if(not(self.serialConnection == None)):
                self.closeConnection()
            return "ERROR"
        # Exception for Serial Object not being available
        except AttributeError:
            logging.error(f"{self.comPort} - Object not available")
            return "ERROR"
            


    def closeConnection(self) -> None:
        """Closes Connection with the Arduino
         Args:
            None
        Returns:
            The Arduinos's response

        Raises:
            none
        """
        logging.info(f"Closing connection with {self.comPort}")
        self.serialConnection.close()
        self.serialConnection = None





if __name__ == '__main__':
    logging.basicConfig(filename = "scTestLog.log",encoding='utf-8', level=logging.DEBUG, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    c = arduinoCom("COM4", 9600)
    c.startConnection()
    while True:
        print(c.sendData("TYPE"))
        time.sleep(1)