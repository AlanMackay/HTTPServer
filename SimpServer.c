/*---------------------------- 
* SimpServer.c 
* Description: http server program
* CSC 361
* Instructor: Kui Wu
* Written by: Alan Mackay
----------------------------*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_STR_LEN 4096
#define MAX_RES_LEN 4096

/* declare the functions used in this program */
int open_communication(int portno);
void perform_http(char *request, char *response, char *directory);
char* read_file(FILE *fp, char *file_buf);

/*-----------main---------------
* This function opens a socket, and listens for any
* client trying to connect, and then forms the connection
* then it reads what the client is requesting 
* and replies with the result of that request
-------------------------------*/
int main(int argc, char **argv){
	/* variable declaration:*/
	int comm_fd, listen_fd; //the file descriptor for the socket used for communication 
	int portno; //the port number to connect to
	char directory[MAX_STR_LEN]; //the directory to look in for the requested file
	char request[MAX_STR_LEN]; // the http request received from the client
	char response[MAX_RES_LEN]; // the response to send back to the client
	
	// check to see if there are enough arguments provided by the user
	if(argc < 3)
	{
		fprintf(stderr,"ERROR: please provide the port number and the location of the file in the program arguments");
		exit(1);
	}
	
	/* grab the port number and the directory from 
	* the program arguments*/
	portno = atoi(argv[1]);
	strncpy(directory, argv[2], MAX_STR_LEN);	
	
	// attempt to open a connection
	listen_fd = open_communication(portno);
	// run until alt+C is pressed by the user
	while(1){
		// listen for an attempted connection from the server
		listen(listen_fd, 10);
		// attempt to accept the connection
		comm_fd = accept(listen_fd, (struct sockaddr*) NULL, NULL);
		// check to see if the connection is successfully made
		if(comm_fd < 0)
		{
			fprintf(stderr,"ERROR: unable to accept communications");
			exit(0);
		}
		
		//write zeros to the memory pointed to by "request" and "response" character strings		
		bzero(request, MAX_STR_LEN);
		bzero(response, MAX_RES_LEN);
		// attempt to read the request from the client 
		if(read(comm_fd,request,MAX_STR_LEN) < 0)
		{
			//the read failed, so report it and exit
			fprintf(stderr,"ERROR: failed to read from the client");
			close(comm_fd);
			exit(1);
		}
		// parse the request and form a response
		perform_http(request, response, directory);
		//send the response back to the client
		if(write(comm_fd,response,MAX_RES_LEN) < 0)
		{
			// the write failed, so report it and exit
			fprintf(stderr,"ERROR: failed to write to the client");
			close(comm_fd);
			exit(1);
		}
		// close the socket used for the communication
		close(comm_fd);
	}
	
}

/*---------------perform_http-----------------
* this funtion parses a client's http request
* and constructs an appropriate response.
* the only http codes that are implemented 
* here are 200 Ok, 404 Not Found, and 501 
* Not Implemented.  if the file requested is
* found, it is opened and appended to the end
* of the response
----------------------------------------------*/

void perform_http(char *request, char *response, char *directory){
	/* variable declaration: */
	char http_code[4]; // holds the http code as a char array
	FILE *fp; // file pointer for the requested file
	char status[100]; // status of the http code
	char method[20]; // the method in the http request
	char location[200]; // the location of the file requested
	char temp_directory[MAX_STR_LEN] = {0}; //to hold the full directory of the desired file
	char http_version[10]; // the version of http being used
	char *token; // a pointer to tokenize the request
	char time_str[100]; // a char array to hold the current time in a specific format
	char *file_buf; // a char array to hold the contents of the requested file
	time_t cur_time; // gets and holds the current time
	
	/* tokenize the request to get "method", "location" and "http_version" */
	token = strtok(request, " ");
	strncpy(method, token, 20);
	
	token = strtok(NULL, " ");
	strncpy(location, token, 200);
	
	token = strtok(NULL, " ");
	strncpy(http_version, strtok(token, "\r"), 10);
	
	/* the location parsed is actually the full uri sent by the client,
	*  must get the actual location of the requested file from the end of the uri
	*/
	token = strtok(location, "/");
	// check if the first part of the uri is "http:" 
	if(strncmp(token, "http:", 5)==0){
		token = strtok(NULL, "/");
	}
	// get the next part of the uri (hostname)
	token = strtok(NULL, "\0");
	/* append a forward slash to "temp_directory"
	*  and then append the location of the requested file */
	strncpy(temp_directory, directory, strlen(directory));
	strncat(temp_directory, "/", 2);
	strncat(temp_directory, token, 200);
	
	//attempt to topen the requested file in the specified location
	fp = fopen(temp_directory, "r");

	// if the porgram fails to open the file, assing http code "404"
	if(fp == NULL)
	{
		strncpy(http_code, "404", 4);
		strncpy(status, "File not found", 15);
	}else
	{//otherwise assign code "200"
		strncpy(http_code, "200", 4);
		strncpy(status, "OK", 3);
	}
	if(strncmp(method, "GET", 4) != 0)
	{//check if the method in the request is implemented in this program
		// if it is not, assign http code "501"
		strncpy(http_code, "501", 4);
		strncpy(status, "Not implemented", 16);
		fp = NULL;
	}
	
	// if the file was successfully opened, read it
	if(fp != NULL)
	{
		if((file_buf = read_file(fp, file_buf)) == NULL)
		{
			// if the file is unreadable, assign http code 404
			strncpy(http_code, "404", 4);
			strncpy(status, "File not found", 15);
		}
		// close the file after reading
		fclose(fp);
	}
	
	// get the current time
	cur_time = time(0);
	// format it for ease of reading
	strftime(time_str, 100, "%a %d, %b %Y %T PST", localtime(&cur_time));
	
	/* start to construct the response with 
	*  information about the request,
	*  the date, and the server being used
	*/
	strncat(response, "---Response Header---\n", 22);
	strncat(response, http_version, strlen(http_version));
	strncat(response, " ", 2);
	strncat(response, http_code, 4);
	strncat(response, " ", 2);
	strncat(response, status, strlen(status));
	strncat(response, "\nDate: ", 8);
	strncat(response, time_str, strlen(time_str));
	strncat(response, "\nServer: Apache/2.2.31 (Unix) mod_jk/1.2.40\n\n", 45);
	
	// if the file was successfully read, concatenate it to the response
	strncat(response, "---Response Body---\n", 21);
	if(fp != NULL)
	{
		strncat(response, file_buf, strlen(file_buf));
	}
	
}

/*--------read_file---------------
* this function reads a file from  a FILE pointer 
* and returns it as a char array
*/

char* read_file(FILE *fp, char *file_buf){
	/* variable declaration: */
	long lSize; // holds the size of the file in bytes
	
	// seek to the end of the file
	fseek(fp, 0, SEEK_END);
	// record the size in bytes
	lSize = ftell(fp);
	//return to the beginning of the file
	rewind(fp);
	
	// allocate an appropriate amount of memory to "file_buf"
	file_buf = calloc(1, lSize+1);
	// return a failure if the memory is not allocated
	if(!file_buf) return NULL;
	
	// read the file, and return NULL if the read fails
	if(fread(file_buf, lSize, 1, fp) != 1) return NULL;
	
	// return the char array holding the contents of the file
	return file_buf;
}

/*----------------open_communication--------------------
* this function attempts to open a socket and use it to 
* connect to a client----------------------------------*/

int open_communication(int portno){
	/* variable declarartion */
	int listen_fd; // describes a socket
	struct sockaddr_in servaddr; // holds info about the connection
	
	//attempt to open a socket
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	
	// if the opening fails, report it and exit
	if(listen_fd < 0){
		fprintf(stderr,"ERROR: unable to open socket");
		exit(0);
	}
	
	// write zeros to the memory pointed to by servaddr
	bzero(&servaddr, sizeof(servaddr));
	
	// populate servaddr with information about the port number
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("10.10.1.100");
	servaddr.sin_port = htons(portno);
	
	//bind the socket using the info in servaddr
	bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	
	// return the listen_fd	
	return listen_fd;
}

