/*------------------------------
* SimpClient.c
* Description: HTTP client program
* CSC 361
* Instructor: Kui Wu
* Written by: Alan Mackay V00752582
-------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <regex.h>

/* Maximum String lengths for sending and receiving strings */
#define MAX_STR_LEN 4096
#define MAX_RES_LEN 4096

/* Function definitions */

void parse_URI(char *uri, char *hostname, int *port, char *identifier);
void perform_http(int sockid, char *identifier, char *host);
int open_connection(char *hostname, int port);
int match_ip(char *hostname);

/*-------Main--------
* the main function takes the arguments provided by the user 
* and parses them, and then opens a connection to the server
* then perfors and http request with that server
*-------------------*/

main(int argc, char **argv)
{
	/* variables to hole the URI, hostname, file to retrieve, socket identifier, and port number */
	char uri[MAX_STR_LEN];
  char hostname[MAX_STR_LEN];
 	char identifier[MAX_STR_LEN];
 	int sockid, port;
	
	/* check to see if there are enough arguments provided */
	if(argc < 2){
		fprintf(stderr,"ERROR: please provide a URI in the program arguments");
		exit(1);
	}
	
	/* copy the argument into the uri variable */
	strncpy(uri, argv[1], MAX_STR_LEN);
	
	/* parse the uri to find the hostname, port and identifier */
	parse_URI(uri, hostname, &port, identifier);
	
	/* open a connection to the server designated by "hostname" */
  sockid = open_connection(hostname, port);
	
	/* form and send an http request to the server */
  perform_http(sockid, identifier, hostname);
  
  /* close the socket once the http has been done */
 	close(sockid);
}

/*------ Parse an "uri" into "hostname" and resource "identifier" --------*/

void parse_URI(char *uri, char *hostname, int *port, char *identifier)
{
	/* variable declaration:
	* token to hold the temporary tokens while tokenizing the uri
	* temphostname to hold the hostname for later tokenizing
	*/
	char *token;
 	char temphostname[MAX_STR_LEN];
	
	/* grab the first section of the uri */
	token = strtok(uri, "/");

	/* the first section may or may not be "http:", so check that */
	if(strncmp(token, "http:", 5)==0){
		/* if it is, grab the next part */
		token = strtok(NULL, "/");
		strncpy(temphostname, token, MAX_STR_LEN);
	}else{
		/* if it is not, then the part we grabbed is the hostname */
		strncpy(temphostname, token, MAX_STR_LEN);
	}
	
	/* grab the next part, which is the identifier, and put a forward slash in front of it */
	strncat(identifier, "/", 1);
	token = strtok(NULL, "\r");

	/* if there is an identifier portion in the uri,
	* it is assigned to the variable "identifier"
	* otherwise, the default identifier of "index.html"
	* is used 
	*/
	if (token != NULL) strncat(identifier, token, MAX_STR_LEN);
	else strncat(identifier, "index.html", 10);
	token = strtok(temphostname, ":");
	strncpy(hostname, token, MAX_STR_LEN);

	/* tokenize the previously grabbed hostname 
	* to see if a port is provided.
	* If one isn't, the default port of 80 is used
	*/
	token = strtok(NULL, ":");
	if(token != NULL){
		*port = atoi(token);
	}else *port = 80;
}

/*------------------------------------*
* connect to a HTTP server using hostname and port,
* and get the resource specified by identifier
*--------------------------------------*/
void perform_http(int sockid, char *identifier, char *host)
{
  	/* Variable declaration:
  	*  http_msg to hold the message being constructed
  	*  buffer to hold the response from the server
  	*  n to hold the result of writing and reading
  	*/
  	char http_msg[MAX_STR_LEN];
  	char buffer[MAX_RES_LEN];
  	int n;
  
  	/* construct the http request with method GET
  	*  hostname and Http version 1.0
  	*/
  	strncat(http_msg, "GET ", 4);
  	strncat(http_msg, host, strlen(host));
  	strncat(http_msg, identifier, strlen(identifier));
  	strncat(http_msg, " ", 1);
  	strncat(http_msg, "HTTP/1.0", 8);

	/* print to the console what is going on */
	printf("---Request Begin---\n");
  	printf("%s\n",http_msg);
  	printf("Host: %s\n", host);
  	printf("Connection: Keep-Alive\n\n");
  
  	/* finish off the request */
  	strncat(http_msg, "\r\n\r\n", 4);
  
  	/* write to the server and see if it works */
  	n = write(sockid, http_msg, MAX_STR_LEN);
  	if(n < 0)
  	{
  		/* if the write failed, print error and close socket and exit */
   		fprintf(stderr,"ERROR writing to socket");
		close(sockid);
  		exit(0);
  	}
  
  	/* tell the user that the request has been sent */
  	printf("---Request End---\n");
  	printf("HTTP request sent, awaiting response...\n\n");
  
  	/* read the response from the server, put it in "buffer" */
  	n = read(sockid, buffer, MAX_RES_LEN);
  	if(n < 0)
  	{
  	/* if the read fails, print error and exit after closing socket */
    fprintf(stderr,"ERROR reading from socket");
	close(sockid);
	exit(0);  
  	}
  	/* print the response from the server */
  	printf("%s\n", buffer);

	/* close the socket */
  	close(sockid);
}

/*---------------------------------------------------------------------------*
 *
 * open_conn() routine. It connects to a remote server on a specified port.
 *
 *---------------------------------------------------------------------------*/

int open_connection(char *hostname, int port)
{
	/* variable declaration */
  	int sockfd;
  	struct sockaddr_in serv_addr;
 	struct hostent *server;

	/* open a socket */
  	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	/* check if the socket failed to open */
  	if (sockfd < 0) fprintf(stderr,"ERROR opening socket\n");
  	
  	/* write zeros to the memory location at serv_addr */
	bzero(&serv_addr,sizeof serv_addr);

	/* assign serv_addr information about the server */
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(port);

	/* use gethostbyname() to get the address info and then copy that to serv_addr */ 
  	server = gethostbyname(hostname);
  	memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
  
	/* try to connect to the server, if the connection fails, print that and exit */
  	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
  	{
  		fprintf(stderr,"ERROR connecting\n");
  		exit(0);
	}
	
	/* return the sockid connected to the server */
  	return sockfd;
}
