from socket import socket, SO_REUSEADDR, SOL_SOCKET
from asyncio import Task, coroutine, get_event_loop, ensure_future


async def handle_client(loop, client_sock):
    print('recv')
    buf = await loop.sock_recv(client_sock, 1024)
    response = str(len(buf)).encode('utf-8')
    print('send')
    await loop.sock_sendall(client_sock, response)
    client_sock.close()

async def serve(loop):
    server_socket = socket()
    server_socket.bind(('localhost', 1234))
    server_socket.listen(5)
    tasks = []
    while True:
        print('accept')
        client_sock, client_addr = await loop.sock_accept(server_socket)
        #handle_client(loop, client_sock)
        #ensure_future(handle_client(loop, client_sock))
        #ensure_future(handle_client(loop, client_sock), loop=loop)
        loop.create_task(handle_client(loop, client_sock))
        #await loop.create_task(handle_client(loop, client_sock))
        #peer_sock.setblocking(0)
        #peer = Peer(self, peer_sock, peer_name)
        #self._peers.append(peer)
        #self.broadcast('Peer %s connected!\n' % (peer.name,))

def main():
    loop = get_event_loop()
    
    #loop.run_until_complete(serve(loop))
    #loop.create_task(serve(loop))
    ensure_future(serve(loop))
    loop.run_forever()

if __name__ == '__main__':
    main()