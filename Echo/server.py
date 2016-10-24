"""
Try multiprocessing?
https://gist.github.com/josiahcarlson/3723597
"""
from threading import Thread
import socket

class EchoThread(Thread):
    def __init__(self, sock):
        self.sock = sock
    def run(self):
        print('recv')
        line = self.sock.recv(1024)
        response = str(len(line))
        print('send "%s"' % response)
        self.sock.send(response)
        self.sock.close()
        

#create an AF_INET (IPv4), SOCK_STREAM (TCP) socket
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# configure the socket to a local/public host
# and a port
serversocket.bind(('localhost', 1234))

# become a server socket, maximum number of queued connections is 5
serversocket.listen(5)

while True:
    # blocking accept connections from outside
    print('accept')
    (clientsocket, address) = serversocket.accept()

    # now do something with the clientsocket
    # in this case, we'll pretend this is a threaded server
    ct = EchoThread(clientsocket)
    ct.run()