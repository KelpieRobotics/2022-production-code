import tkinter as tk
from tkinter import ttk
import threading

# https://stackoverflow.com/questions/459083/how-do-you-run-your-own-code-alongside-tkinters-event-loop

class captureOverlay (threading.Thread):
    def __init__(self) -> None:
        threading.Thread.__init__(self)
        self.start()
    def callback(self):
        self.root.quit()

    def run(self):
        self.app = tk.Tk()
        self.app.title('Kelpie Data Capture Overlay')
        self.app.geometry('1920x540')
        self.app.configure(bg='#00B140')
        self.app.columnconfigure(0, weight=1)
        self.app.columnconfigure(1, weight=1)

        # Humidity
        self.humidity_label = ttk.Label(self.app, text="Humidity: READY", font=('Arial',15))
        self.humidity_label.grid(column=0, row=0, sticky=tk.W, padx=5, pady=5)


        self.temperature_label = ttk.Label(self.app, text="Temperature: READY", font=('Arial',15))
        self.temperature_label.grid(column=1, row=0, sticky=tk.W, padx=5, pady=5)

        self.power_info_label = ttk.Label(self.app, text="Input: READY\tOut: READY", font=('Arial',15))
        self.power_info_label.grid(column=2, row=0, sticky=tk.W, padx=5, pady=5)


        self.sensorError = ttk.Label(self.app, text="", font=('Arial',50))
        self.sensorError.grid(column=2, row=1, sticky=tk.E, padx=5, pady=5)
        self.motorError = ttk.Label(self.app, text="", font=('Arial',50))
        self.motorError.grid(column=2, row=2, sticky=tk.E, padx=5, pady=5)

        self.leakError = ttk.Label(self.app, text="",font=('Arial',75))
        self.leakError.grid(column=1, row=3, sticky=tk.S, padx=5, pady=5)
        self.app.mainloop()
    
    def updateMotorState(self, newData):
        if newData == "OK":
            self.motorError['text'] = ""
        else:
            self.motorError['text'] = "MOTOR COM LOST"


    def updateData(self, newData):
        if newData[1] == "ERROR":
            self.sensorError['text'] = "SENSOR COM LOST"
            self.humidity_label['text'] = ""
            self.temperature_label['text'] = ""
            self.power_info_label['text'] = ""

        else:
            if newData[3] == '1':
                self.leakError['text'] = "LEAK\nDETECTED"
            else:
                self.leakError['text'] = ""
            self.sensorError['text'] = ""
            self.humidity_label['text'] = f"Humidity: {newData[1]}%"
            self.temperature_label['text'] = f"Enc. Temperature: {newData[2]}C PMBus: {newData[7]}"
            self.power_info_label['text'] = f"V_in: {newData[4]}V V_out: {newData[5]}V {newData[6]}A {newData[8]}W"
        
if __name__ == '__main__':
    captureOverlay()