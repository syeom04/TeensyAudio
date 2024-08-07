import serial.tools.list_ports

ports = serial.tools.list_ports.comports()
serialInst = serial.Serial()

# Current port: COM5 - USB 직렬 장치(COM5)
portList = list()
for onePort in ports:
    print("Current port: " + str(onePort))
    portList.append(str(onePort))
print(portList)
val = input("Select the current port (ENTER INTEGER ONLY)\n")
for x in range(0, len(portList)):
    if portList[x].startswith("COM" + str(val)):
        portVar = "COM" + str(val)
        print("You have selected " + str(portVar))
        break

serialInst.baudrate = 9600
serialInst.port = portVar
serialInst.open()

try:
    while True:
        if serialInst.in_waiting:
            packet = serialInst.readline()
            print(packet.decode('utf').rstrip('\n'))
except KeyboardInterrupt:
    print("Keyboard Interrupt. Exiting...")
    serialInst.close()
