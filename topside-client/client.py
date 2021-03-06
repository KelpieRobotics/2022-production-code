from tcpClient import tcpClient
from gamePad import XboxController
from dataLogger import dataLogger
import threading
import logging
import time
from datetime import datetime

class main:
    def __init__(self, serverIP, serverPort) -> None:
        # Server variabeles
        self.serverIP = serverIP
        self.motorPort = serverPort
        self.sensorPort = serverPort + 1
        self.runThreading = True
        self.sendComPortCommands = False
        print("Starting...")
        logging.debug("Initializing Main - IP {self.serverIP}, MotorTCP: {self.motorPort}, SensorTCP: {self.sensorPort}")
        # Objects 
        self.gamePad = XboxController()
        self.tcp_motors = None
        self.tcp_sensors = None
        self.dataSave = dataLogger(["Time(s)","Humidity(%)","Enclosure Temperature(C)","Leak","Vin(Vrms)","Vout(Vrms)","Current Out(A)", "PMBus Temperature(C)", "Power Out(W)"])
        # print("READY")



    def startSockets(self):
        logging.info("Starting Sockets")
        successfulConection = False
        while not successfulConection:
            try:
                self.tcp_motors = tcpClient(self.serverIP, self.motorPort)
                self.tcp_sensors = tcpClient(self.serverIP, self.sensorPort)
                self.tcp_motors.startConnection()
                self.tcp_sensors.startConnection()
                successfulConection = True
            except ConnectionRefusedError as e:
                print(f"Connection Refused Error: {e}")
                logging.debug(f"Starting Socket - Connection Refused Error: {e}")
                successfulConection = False
            except OSError as e:
                print(f"OS Error: {e}")
                logging.debug(f"Starting Socket - OS Error: {e}")
                successfulConection = False
            except Exception as e:
                print(f"Error: {e}")
                logging.debug(f"Starting Socket - Error: {e}")
                successfulConection = False
            # Retry connection
            time.sleep(1)
        print("Connected")



    def startThreads(self):
        logging.info("Starting Threads")
        try:
            motorThread = threading.Thread(target=self.__Thread_process_joystick__)
            sensorThread = threading.Thread(target=self.__Thread_process_data__)
            motorThread.daemon = True
            sensorThread.daemon = True
            motorThread.start()
            sensorThread.start()
            while motorThread.is_alive() and sensorThread.is_alive():
                motorThread.join(1)
                sensorThread.join(1)
        except KeyboardInterrupt:
            logging.warning("Keyboard Interrupt was preformed, closing threads")


    def __Thread_process_joystick__(self):
        logging.info("Starting Joystick thread")
        while True:
            if self.runThreading:
                gamePadTriggers = self.gamePad.readTriggers()
                gamePadSticks = self.gamePad.readAnalogSticks()
                gamePadButtons = self.gamePad.readMainButtons()

                # COM Port is active
                if self.sendComPortCommands:
                    # Use the following format if using both joysticks
                    # formmatedData = f"S\t{gamePadSticks[1]},{gamePadSticks[2]}\t{gamePadSticks[4]},{gamePadSticks[5]}\t{gamePadTriggers[0]},{gamePadTriggers[1]},{gamePadTriggers[2]},{gamePadTriggers[3]}"
                    # Use the following format if using dpad and right joystick
                    formmatedData = f"D\t{gamePadButtons[6]},{gamePadButtons[7]}\t{gamePadSticks[4]},{gamePadSticks[5]}\t{gamePadTriggers[0]},{gamePadTriggers[1]},{gamePadTriggers[2]},{gamePadTriggers[3]}"

                    print(self.tcp_motors.sendData("MOT"+formmatedData))
                # SELECT + START to stop threading
                if gamePadButtons[4] == 1 and gamePadButtons[5]== 1:
                    print("STOPING THREADS")
                    self.runThreading = False



                # Start Button to start sending data to arduinos
                if gamePadButtons[3] == 1:
                    print("START COMPORT")
                    self.sendComPortCommands = True

            else:
                break


    
    def __Thread_process_data__(self):
        logging.info("Starting DATA thread")
        lastDataCap = self.current_milli_time()
        while True:
            if self.runThreading  and self.sendComPortCommands and(self.current_milli_time() - lastDataCap >= 50):
                # Fetch Data from the sensor server thread
                returnedData = self.tcp_sensors.sendData("SEN")
                # Parse Data
                returnedDataCSV = returnedData.replace("\t",",")
                returnedDataCSV = f"{datetime.now()},{returnedDataCSV}"
                returnedDataList = returnedDataCSV.split(",")
                print(returnedDataList)
                self.dataSave.writeCSVString(returnedDataCSV)
                lastDataCap = self.current_milli_time()
            elif (self.runThreading == False):
                break
    
    def current_milli_time(self):
        return round(time.time() * 1000)



if __name__ == '__main__':
    logging.basicConfig(filename = "log.log", encoding='utf-8', level=logging.DEBUG, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    client = main("10.10.2.5", 8000)
    # client = main("localhost", 8000)
    client.startSockets()
    client.startThreads()

