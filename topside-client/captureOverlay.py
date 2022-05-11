import tkinter as tk
from tkinter import ttk

# https://stackoverflow.com/questions/459083/how-do-you-run-your-own-code-alongside-tkinters-event-loop

class captureOverlay:
    def __init__(self) -> None:
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


        # erroe1 = ttk.Label(self.app, text="", font=('Arial',50))
        # erroe1.grid(column=2, row=1, sticky=tk.E, padx=5, pady=5)
        # erroe2 = ttk.Label(self.app, text="", font=('Arial',50))
        # erroe2.grid(column=2, row=2, sticky=tk.E, padx=5, pady=5)

        # leak = ttk.Label(self.app, text="",font=('Arial',75))
        # leak.grid(column=1, row=3, sticky=tk.S, padx=5, pady=5)

        
    def startLoop(self):
        self.app.mainloop()
    def updateData(self, inut):
        self.temperature_label['text'] = input
if __name__ == '__main__':
    captureOverlay().startLoop()
