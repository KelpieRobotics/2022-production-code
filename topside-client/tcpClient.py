import socket
import logging
# TODO: THIS NEEDS ERROR HANDLING
class tcpClient:
    def __init__(self, serverIP: str, serverPort: int) -> None:
        """None

        Args:
            serverIP: IP address of the communication Server
            serverPort: port of the communication server

        Returns:
            null

        Raises:
            null
        """
        
        self.serverIP = serverIP
        self.serverPort = serverPort
        logging.debug(f"Initializing Client - IP {self.serverIP}:{self.serverPort}")

    def startConnection(self):
        """Establishes Connection with the Server

        Args:
            null

        Returns:
            null

        Raises:
            null
        """
        logging.info(f"Attempting to establish connection with {self.serverIP}:{self.serverPort}")
        self.connection = socket.socket()
        self.connection.connect((self.serverIP, self.serverPort))

    def sendData(self, data: str) -> str:
        """Sends data to the server

        Args:
            data: The data that needs to be sent to the server in string format

        Returns:
            The server's response

        Raises:
            null
        """
        self.connection.send(data.encode('utf-8'))
        serverFeedback = self.connection.recv(1024).decode('utf-8')
        return serverFeedback

    def closeConnection(self) -> None:
        """Closes the connection to the server

        Args:
            null

        Returns:
            null

        Raises:
            null
        """
        logging.info(f"Closing socket for {self.serverIP}:{self.serverPort}")
        self.connection.close()

