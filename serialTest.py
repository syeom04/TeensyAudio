import serial.tools.list_ports
import re
import numpy as np
import matplotlib.pyplot as plt

# List all available ports
ports = serial.tools.list_ports.comports()
portList = list()
for onePort in ports:
    print("Current port: " + str(onePort))
    portList.append(str(onePort))
print(portList)
val = input("Select the current port (ENTER INTEGER ONLY)\n")
for x in range(len(portList)):
    if portList[x].startswith("COM" + str(val)):
        portVar = "COM" + str(val)
        print("You have selected " + str(portVar))
        break

# Serial configuration
serialInst = serial.Serial()
serialInst.baudrate = 9600
serialInst.port = portVar
serialInst.open()

fft_size = 1024
sampling_rate = 96000
frequencies = np.fft.fftfreq(fft_size, d=1./sampling_rate)[:fft_size//2]  # Use only positive frequencies
fft_values = []

# Initialize the plot
plt.ion()
fig, ax = plt.subplots()
line, = ax.plot(frequencies, np.zeros(len(frequencies)), 'b-')
ax.set_xlim(0, frequencies[-1])
ax.set_ylim(0, 1)  # Adjust based on expected magnitude
ax.set_title("FFT Visualization")
ax.set_xlabel("Frequency (Hz)")
ax.set_ylabel("Magnitude")
ax.grid(True)

try:
    while True:
        if serialInst.in_waiting:
            packet = serialInst.readline().decode('utf-8').strip()

            if packet.startswith("FFT signal now:"):
                numbers = re.findall(r"-?\d+\.\d+", packet)
                fft_values.extend([float(num) for num in numbers])
                
                if len(fft_values) >= fft_size:
                    fft_values = fft_values[-fft_size:]
                    
                    # Calculate magnitude
                    magnitude = np.abs(np.fft.fft(fft_values)[:fft_size//2])
                    
                    # Update the plot
                    line.set_ydata(magnitude)
                    ax.relim()
                    ax.autoscale_view()
                    plt.draw()
                    plt.pause(0.001)
                    
                    fft_values = []

except KeyboardInterrupt:
    print("Keyboard Interrupt. Exiting...")
    serialInst.close()
    plt.ioff()
    plt.show()