var net = require('net');

var server = net.createServer((socket) => {
    //socket.end('goodbye\n');
    console.log('New connection!');
    socket.on('data', (data) => {
        console.log('Got: "' + data + '"');
        var response = data.length.toString();
        socket.end(response);
    });
}).on('error', (err) => {
    // handle errors here
    throw err;
});

// grab a random port.
server.listen({host: "localhost", port: 1234}, () => {
    console.log('opened server on', server.address());
});