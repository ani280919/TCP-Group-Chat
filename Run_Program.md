To run the server program
/*Uses thread for Multiple clients+asynchronous+non-blocking*/
terminal :

gcc chatserver.c -pthread
./a.out <port no>


eg:
gcc chatserver.c -pthread
./a.out 9090


------------------------------------

To run the client program
/*Uses thread for Multiple clients+asynchronous+non-blocking*/
terminal :

gcc chatclient.c -pthread
./a.out <port no>


eg:
gcc chatclient.c -pthread
./a.out 9090

