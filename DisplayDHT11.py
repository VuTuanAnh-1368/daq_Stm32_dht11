import tkinter as tk
from tkinter import ttk
import serial
import serial.tools.list_ports

class SerialApp:
    def __init__(self, master):
        self.master = master
        self.master.title("DAQ_STM32")
        self.master.geometry("600x600")  

        large_font = ('Verdana', 12)  
        label_bg = 'lightblue' 
        style = ttk.Style()
        style.configure('TButton', background='lightgreen', font=('Verdana', 12))
        style.configure('TLabel1', background='lightgreen', font=('Verdana', 30), foreground='red') 
        style.configure('Large.TLabel', font=('Verdana', 16), foreground='red') 
       
        self.port_label = ttk.Label(master, text="Select COM Port:")
        self.port_label.pack(pady=5)

        self.port_combo = ttk.Combobox(master, width=15)
        self.port_combo.pack(pady=5)
        self.refresh_ports()

        self.baud_rate_label = ttk.Label(master, text="Select Baud Rate:")
        self.baud_rate_label.pack(pady=5)

        self.baud_rate_combo = ttk.Combobox(master, width=15)
        self.baud_rate_combo['values'] = [9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600]
        self.baud_rate_combo.set(115200)  
        self.baud_rate_combo.pack(pady=5)

        self.connect_button = ttk.Button(master, text="Connect", command=self.connect_to_port, style='TButton')
        self.connect_button.pack(pady=5)

        self.refresh_button = ttk.Button(master, text="Refresh Ports", command=self.refresh_ports)
        self.refresh_button.pack(pady=5)

        self.data_label = ttk.Label(master, text="Waiting for data...", style='Large.TLabel', wraplength=580)
        self.data_label.pack(pady=40, padx=20)


        self.ser = None

    def refresh_ports(self):
        """Refresh the list of COM ports."""
        ports = serial.tools.list_ports.comports()
        self.port_combo['values'] = [port.device for port in ports]

    def connect_to_port(self):
        """Connect to the selected COM port."""
        com_port = self.port_combo.get()
        baud_rate = int(self.baud_rate_combo.get()) 
        try:
            self.ser = serial.Serial(com_port, baud_rate, timeout=1)
            self.read_data()
        except serial.SerialException as e:
            self.data_label['text'] = str(e)

    def read_data(self):
        """Read data from the serial port."""
        if self.ser and self.ser.isOpen():
            while True:
                data = self.ser.readline()
                if data:
                    decoded_data = data.decode('utf-8').rstrip()
                    self.data_label['text'] = decoded_data
                self.master.update()  

root = tk.Tk()
app = SerialApp(root)
root.mainloop()
