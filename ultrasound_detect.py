import sys
import re
import numpy as np
import serial.tools.list_ports
import pyqtgraph as pg
from PyQt6 import QtWidgets, QtCore

class FFTVisualizer:
    def __init__(self):
        pg.setConfigOptions(antialias=True)
        
        # Initialize Qt Application
        self.app = QtWidgets.QApplication(sys.argv)
        
        # Create a GraphicsLayoutWidget
        self.win = pg.GraphicsLayoutWidget()
        self.win.setWindowTitle('FFT Visualization')
        self.win.setGeometry(100, 100, 1000, 600)
        
        # Create plot
        self.plot = self.win.addPlot(title='FFT Visualization')
        self.plot.setLabel('left', 'Magnitude')
        self.plot.setLabel('bottom', 'Frequency (Hz)')
        self.plot.setXRange(0, 96000)  # Adjust to the new sampling rate
        self.plot.setYRange(0, 1)
        
        # Create a line plot item
        self.line = self.plot.plot(pen='b')
        
        # Fix the y-axis range
        self.y_min = 0
        self.y_max = 1
        self.plot.setYRange(self.y_min, self.y_max)
        
        # Serial configuration
        ports = serial.tools.list_ports.comports()
        portList = [str(onePort) for onePort in ports]
        print("Available ports:", portList)
        
        if len(portList) == 1:
            # Use the only available port
            current_port = str(portList[0]).split(" ")[0]
            print("Automatically selected port:", current_port)
            self.serialInst = serial.Serial(port=current_port, baudrate=9600)
        else:
            # When there are multiple ports, ask the user to select one
            val = input("Select the current port\nFor example, if the port is COM5, simply enter '5'\n")
            portVar = "COM" + str(val)
            print("You have selected " + str(portVar))
            self.serialInst = serial.Serial(port=portVar, baudrate=9600)
            
        self.fft_size = 4096  # Increase FFT size for better resolution
        self.sampling_rate = 192000  # Increase sampling rate for higher frequency detection
        self.frequencies = np.fft.fftfreq(self.fft_size, d=1./self.sampling_rate)[:self.fft_size//2]
        
        # Initialize ring buffer for FFT values
        self.fft_values = np.zeros(self.fft_size)
        self.index = 0
        
        # Set up the timer
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.timer.start() 
        
        # Show the window
        self.win.show()

    def update_plot(self):
        if self.serialInst.in_waiting:
            packet = self.serialInst.readline().decode('utf-8').strip()
            
            if packet.startswith("FFT signal now:"):
                numbers = re.findall(r"-?\d+\.\d+", packet)
                new_data = np.array([float(num) for num in numbers])
                
                if new_data.size == 0:
                    return
                
                if new_data.size > self.fft_size:
                    new_data = new_data[-self.fft_size:]  # Truncate to fit

                # Handle the case where the new data does not fill the buffer completely
                end_index = (self.index + len(new_data)) % self.fft_size
                if end_index > self.index:
                    self.fft_values[self.index:end_index] = new_data
                else:
                    part1_len = self.fft_size - self.index
                    self.fft_values[self.index:] = new_data[:part1_len]
                    self.fft_values[:end_index] = new_data[part1_len:]
                
                self.index = end_index
                
                # Perform FFT and update plot
                magnitude = np.abs(np.fft.fft(self.fft_values)[:self.fft_size//2])
                self.line.setData(self.frequencies, magnitude)
    
    def run(self):
        QtWidgets.QApplication.instance().exec()

if __name__ == "__main__":
    visualizer = FFTVisualizer()
    visualizer.run()
