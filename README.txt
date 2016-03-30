SimpServer.c and SimpClient.c
Alan Mackay
CSC 361

Compile the programs by running the MakeFile provided (Type the command "make" into the terminal in the directory containing the source files and the MakeFile)

Run SimpServer.c by using the command "./SimpServer <portno> <directory>" where 
<portno> is the port number you want the server to run on and <directory> is the directory you want
the client to search for files in. <directory> must not have a forward-slash as it's last character.

Run SimpClient.c by using the command 
"./SimpClient http://<hostname>:<portno>/<filepath>" where <hostname> is the the name or IP address of the server you wish to connect to (If you are trying to connect to SimpServer, use the IP address 10.10.1.100. SimpServer must already be running) <portno> is the port number you wish to use for the connection (again if you are trying to connect to SimpServer, you must use the same port number for SimpClient as you are using for SimpServer) and <filepath> is the path and or filename of the file you wish to retrieve from the http server. both "http://" and <portno> are optional, and if <portno> is left out, the client will default to port 80,

The code for the tcp server portion of each file is based on the programs tcp_server.c and tcp_client.c which were provided in lab2 of this course (CSC361).
