import serial
import time

# XBee setting
serdev = '/dev/ttyUSB1'
s = serial.Serial(serdev, 9600)

s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())

s.write("ATMY 0x164\r\n".encode())
char = s.read(3)
print("Set MY 0x164.")
print(char.decode())

s.write("ATDL 0x264\r\n".encode())
char = s.read(3)
print("Set DL 0x264.")
print(char.decode())

s.write("ATID 0x1\r\n".encode())
char = s.read(3)
print("Set PAN ID 0x1.")
print(char.decode())

s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())

s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())

s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())

s.write("ATCN\r\n".encode())
char = s.read(3)
print("Exit AT mode.")
print(char.decode())

print("start sending RPC")
while True:
    # send RPC to remote

    s.write("/Cargo/run 20 20 \n\r".encode())
    time.sleep(5)

    s.write("/myled2/write 1 \n\r".encode())
    time.sleep(1)

    s.write("/Cargo/run -20 20 \n\r".encode())
    time.sleep(5)

    s.write("/myled3/write 1 \n\r".encode())
    time.sleep(1)

    s.write("/Cargo/run -20 -20 \n\r".encode())
    time.sleep(5)

    s.write("/myled3/write 0 \n\r".encode())
    time.sleep(1)

    s.write("/Cargo/run 20 -20 \n\r".encode())
    time.sleep(5)

    s.write("/myled2/write 0 \r".encode())
    time.sleep(1)


s.close()
