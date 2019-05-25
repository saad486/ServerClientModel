# Multithreaded Server Client Model
It is a multithreaded server and client. It is developed in language C and it uses linux system APIs. 


## how to use this on linux terminal 
```
1) Open two terminals in directory you have cloned.
2) Compile the program with the gcc compiler.
3) execute the server with "./server" (if I have compiled it like :gcc -pthread server.c -o server).
4) Server will print the port number it is listening on.
5) Compile the client in the similar manner.
6) In client,the port number printed by the server is passed as an argument preceeded by ip address.
7) Like ./client localhost/127.0.0.1 ##### (ip would be different if server and client are running on a separate machines.Type the relevant the port number in place of #).
8) Commands guide is given in the help command. type help.
```
