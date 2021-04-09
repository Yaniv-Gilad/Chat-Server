chat server

=== Description ===

Program files:
chatserver.c - file that implement a tcp chat server.
lib.h - header file of struct and function
lib.c - implement of header file (message queue, list of clients...)


implement a simple chat server using TCP and select().
The server waits for clients’ request. Once the server reads a message from the client, it reads it
till a new line appears.
The server should be run like this:
./server <port>
The server can talk with many clients, each on a different socket.
The server gets a message from the client and send it to all clients except the one who sends it.
The server assigns names to each client, the name is ‘guest<sd>’ where sd is the socket
descriptor assigned to this client after ‘accept()’ returns.

to exit the program please press ctrl+c.


main functions:
	read_messages - check if there is something to read from one of the clients.
    add_message - add message to the queue messages.
    send_messages - send messages to the relevent clients from the queue.

compile - gcc chatserver.c lib.c -o server
run - ./server <port>





	




