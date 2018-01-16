# network-programming

This repo contains some of my networking related projects

## Socket Programming

1. Created a server for multi-clients to connect.
2. Clients can send the texts to the server, and the server can send the content back.
3. Server can support user defined API for client. 
ig. if client sent "cat tmp.txt" to the server, client will get the content of "tmp.txt" on server dir

### how to use:
1. type `make server` and `make client` to compile both of the src files
2. start the server with port number `./server PORT_NUMBER`
3. client should connect server with server's ip `./client SERVER_IP PORT_NUMBER`
4. I defined `cat` function under server, feel free to extend the server's features.
5. Have fun!


![ScreenShot](https://github.com/boalinlai/network-programming/blob/master/img/socket_prog_pic.png)
