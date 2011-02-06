// Node.js socket server by Kevin Ewe
// A simple socket server that's acting as a bridge between a NITE client app and a UI layer (e.g. flash)
// Inspired by Blitzlabs http://labs.blitzagency.com/?p=2634
//
// to start the server, node socket-server.js
//

var net = require('net');
var dataToSend;
var socketServer; 

net.createServer(function (socket){
  socketServer = socket;
  socket.write("Socket server ready...");
  setInterval ( writeToSocketServer, 33 );
  socket.on("data", function (data){
    dataToSend = data;
	});
	socket.on("disconnect", function(){ 
		console.log("bye");
	});
	socket.on("error", function(err){
		console.log(err);
	});
}).listen(8124, "127.0.0.1");

console.log('Server is running at http://127.0.0.1:8124/');



function writeToSocketServer() {
	if (dataToSend) {
		socketServer.write(dataToSend);
	}
}
