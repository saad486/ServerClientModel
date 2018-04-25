# ServerClientModel
Version1 changes the feature of writting list directly on a temporary buffer and writing the buffer to the socket(Server side). Instead we are now passing the structure of list to the client, who having the similar structure is catching and storing the list on its side. And we are printing the list on client side.

# how to use this on linux terminal
1) Open two terminals in directory you have cloned.
2) Compile the program with the gcc compiler.
3) execute the server with "./server" (if I have compiled it like :gcc -o server server.c).
4) Compile the client in the similar manner.
5) In client,the port number printed by the server is passed as an argument preceeded by ip address.
6) Like ./client localhost/127.0.0.1 ##### (ip would be different if server and client are running on a separate machines).
7) Commands guide is given in the help command. type help.
