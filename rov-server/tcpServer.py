from cmath import log
from serialHandler import serialHandler, unableToConnectToArduino, unassignedArduinoType
import threading
import socket
import logging




class tcpServer:
    def __init__(self, serverIP: str, serverPort: int) -> None:
        """Server side of the ROV comunication protocol, run on raspberry pi

        Args:
            serverIP: IP address of this computer (device hosting the server)
            serverPort: port that the server will be avaliable on

        Returns:
            serverIP:
            serverPort: 

        Raises:
            null
        """

        # Server variabeles
        self.serverIP = serverIP
        self.motorPort = serverPort
        self.sensorPort = serverPort + 1
        self.runThreading = True
        logging.info(f"Initializing Server - IP {self.serverIP}, MotorTCP: {self.motorPort}, SensorTCP: {self.sensorPort}")
        # Objects 
        self.serialH = None
        self.tcp_motors = None
        self.tcp_sensors = None

        # If connection with arduino's cannot be established, there is no point continuing

        ############## Connect to serial handler (start communication with arduinos) ##############
        self.serialH = serialHandler()
        try:
            self.serialH.autoConnect()
        except unassignedArduinoType as e:
            logging.error(f"{e}")
            print("There was an issue connecting to the arduinos check the logs for more information")
            self.stopAllServices()
        except unableToConnectToArduino as e:
            logging.error(f"{e}")
            print("There was an issue connecting to the arduinos check the logs for more information")
            self.stopAllServices() 
        ############## End of setup for aduino communicaiton ##############

    
    def configureServer(self, port):
        """Starts the server associated with an ip
        Args:
            id: the server ip
        Returns:
            server object for port
        """
        try:
            logging.info(f"Starting server on port {port}")
            server = socket.socket()
            server.bind((self.serverIP, int(port)))
            server.listen(1)
            return server
            
        # If a port is not available or already in use
        except socket.error as e:
            logging.error(f"Unable to start {id} server due to {e}")
            self.stopAllServices()


    
    def tcpServerListen(self, serverObj):
        """Listens for TCP packets on the specified Server

        Args:
            server: server object

        Returns:
            null

        Raises:
            null
        """
        client_socket = None
        while self.runThreading:
            try: 
                if client_socket == None:
                    client_socket, clientAddress = serverObj.accept()
                    logging.debug(f"Starting listen for {serverObj}")
                    logging.info(f"Connection from: {clientAddress}")


                # Received Data from Client
                receivedData = client_socket.recv(1024).decode('utf-8')
                if not receivedData:
                    break
                # Processes Data and Returns Data to Client
                transmitData = self.processReceivedData(receivedData)
                client_socket.send(transmitData.encode('utf-8'))
            except socket.error as e: 
                logging.error(f"{serverObj} - Socket Error Occurred {e}")
                client_socket = None
            except Exception as e:
                logging.error(f"{serverObj} - An Error has occurred {e}")
   
        client_socket.close()
        logging.debug(f"Server Closed: {serverObj}")

    def startSockets(self):
        logging.info("Starting Sockets")
        self.tcp_motors = self.configureServer(self.motorPort)
        self.tcp_sensors = self.configureServer(self.sensorPort)

    def startListeningThreads(self):
        logging.info("Starting Threads")
        try:
            thread_tcp_motor = threading.Thread(target=self.tcpServerListen,args=(self.tcp_motors,))
            thread_tcp_sensor = threading.Thread(target=self.tcpServerListen,args=(self.tcp_sensors,))
            thread_tcp_motor.daemon = True
            thread_tcp_sensor.daemon = True
            thread_tcp_motor.start()
            thread_tcp_sensor.start()
            while thread_tcp_motor.is_alive() and thread_tcp_sensor.is_alive():
                thread_tcp_motor.join(1)
                thread_tcp_sensor.join(1)
        except KeyboardInterrupt:
            logging.warning("Keyboard Interrupt was preformed, closing threads")

            self.stopAllServices()

    def processReceivedData(self, data):
        """Processes Data received by TCP server and directs it to appropriate location

        Args:
            data: raw data received from TCP

        Returns:
            data based on command issued.

        Raises:
            null
        """
        # Commands Starting with MOT are for motor control, thus they are sent to the motor arduino
        if data.startswith("MOT"):
            return self.serialH.sendMotorCommands(data[3:])
        # Commands Starting with SEN are for sensors, thus they are sent to the sensor arduino
        elif data.startswith("SEN"):
            return self.serialH.sendSensorCommands("GET")
        elif data.startswith("STOP"): # FIXME: This no longer stops the threads
            logging.warning("Received Stop Command")
            self.runThreading = False
            self.stopAllServices()
        else:
            return "INVALID COMMAND"






    def closeTCPsockets(self):
        logging.debug("Closing all TCP Connections")
        self.tcp_motors.close()
        self.tcp_sensors.close()


    def stopAllServices(self):
        logging.warning("Stopping all services")
        self.serialH.closeAllConnections()
        self.closeTCPsockets()
        quit()


if __name__ == '__main__':
    logging.basicConfig(filename = "log.log", encoding='utf-8', level=logging.DEBUG, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
    c = tcpServer("10.10.1.154",8010)
    c.startSockets()
    c.startListeningThreads()
