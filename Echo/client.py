# Echo client program
import socket
import time

HOST = 'localhost'
PORT = 1234
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("connect")
s.connect((HOST, PORT))

print("sleep")
time.sleep(1)

print("send")
s.sendall('Hello, world\n')
print("recv")
data = s.recv(1024)
s.close()
print('Received %r' % data)
