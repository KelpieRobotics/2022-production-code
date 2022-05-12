from datetime import datetime
import csv
import logging


class dataLogger:
    def __init__(self, dataCategories: list) -> None:
        logging_path = datetime.now().strftime("data_logs/log-%b-%d-%Y-%H-%M.csv")
        logging.info(f"Preparing data logging file at {logging_path}")
        self.csvFile = open(logging_path, "w", newline='')
        self.csvWriter = csv.writer(self.csvFile, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        self.writeData(dataCategories)
    def writeData(self, data:list) -> None:
        """Writes line of data to the csv file

        Args:
            data: A list containing the data to be written to the file.

        Returns:
            null

        Raises:
            null
        """
        self.csvWriter.writerow(data)

    def writeCSVString(self, data:str) -> None:
        """Writes line of data to the csv file from a string csv

        Args:
            data: A string csv containing the data to be written to the file.

        Returns:
            null

        Raises:
            null
        """
        formattedData = data.split(",")
        self.writeData(formattedData)

    def closeFile(self) -> None:
        """Closes the file

        Args:
            null

        Returns:
            null

        Raises:
            null
        """
        logging.debug("Closing logging file")
        self.csvFile.close()


