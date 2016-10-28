import socket
import select

#create an AF_INET (IPv4), SOCK_STREAM (TCP) socket
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# configure the socket to a local/public host
# and a port
serversocket.bind(('localhost', 1234))

# become a server socket, maximum number of queued connections is 5
serversocket.listen(5)

active_sockets = [serversocket]

while True:
    print('select')
    input_ready, output_ready, except_ready = select.select(active_sockets, [], []) 
    for sock in input_ready: 
        if sock == serversocket: 
            # handle the server socket 
            print('accept')
            client_socket, address = serversocket.accept() 
            active_sockets.append(client_socket) 
        else: 
            # handle data client sent
            line = sock.recv(1024)
            response = str(len(line))
            print('send "%s"' % response)
            sock.send(response)
            sock.close()
            active_sockets.remove(sock) 

