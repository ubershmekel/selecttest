import random
import socket
import time

import tqdm

HOST = 'localhost'
PORT = 1234

def connect():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    return s

def main(n=1000):
    socks = []
    for i in tqdm.tqdm(range(n)):
        socks.append(connect())
        time.sleep(0.001)
    
    print('sock count: %d' % len(socks))
    time.sleep(1)
    lucky_connection = random.choice(socks)
    
    msg = 'Hello, world\n'
    expected_res = str(len(msg))

    start = time.clock()
    lucky_connection.sendall(msg)
    data = lucky_connection.recv(1024)
    duration = time.clock() - start
    print('Received %r' % data)
    print('Time: %g' % duration)

if __name__ == "__main__":
    main()