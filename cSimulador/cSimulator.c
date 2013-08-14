#include "sllp_server.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define BUFSIZE 1024
void error(char *msg) {
  perror(msg);
  exit(1);
}
int main (){
	struct sllp_var dummy[6];
	enum sllp_err err;
	uint8_t dble[8] = {0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	uint8_t mb[1] = { 0x03 }; 
	sllp_server_t *sllp = sllp_server_new();

	dummy[0].info.id = 0;
	dummy[0].info.writable = true;
	dummy[0].info.size = 8;
	dummy[0].data = dble;

	err = sllp_register_variable(sllp,&dummy[0]);
	
	dummy[1].info.id = 0;
	dummy[1].info.writable = true;
	dummy[1].info.size = 8;
	dummy[1].data = dble;
	
	err = sllp_register_variable(sllp,&dummy[1]);
	
	dummy[2].info.id = 0;
	dummy[2].info.writable = true;
	dummy[2].info.size = 8;
	dummy[2].data = dble;
	
	err = sllp_register_variable(sllp,&dummy[2]);
	
	dummy[3].info.id = 0;
	dummy[3].info.writable = true;
	dummy[3].info.size = 8;
	dummy[3].data = dble;
	
	err = sllp_register_variable(sllp,&dummy[3]);
	
	dummy[4].info.id = 0;
	dummy[4].info.writable = true;
	dummy[4].info.size = 8;
	dummy[4].data = dble;
	
	err = sllp_register_variable(sllp,&dummy[4]);
	
	dummy[5].info.id = 0;
	dummy[5].info.writable = true;
	dummy[5].info.size = 8;
	dummy[5].data = mb;
	
	err = sllp_register_variable(sllp,&dummy[5]);

	int parentfd; /* parent socket */
	int childfd; /* child socket */
	int portno; /* port to listen on */
	int clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	char buf[BUFSIZE]; /* message buffer */
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */

	portno = 6791;

	 /* 
	  * socket: create the parent socket 
	  */
	parentfd = socket(AF_INET, SOCK_STREAM, 0);
	if (parentfd < 0) 
		error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets 
	* us rerun the server immediately after we kill it; 
	* otherwise we have to wait about 20 secs. 
	* Eliminates "ERROR on binding: Address already in use" error. 
	*/
	optval = 1;
	setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

	/*
	 * build the server's Internet address
	 */
	bzero((char *) &serveraddr, sizeof(serveraddr));

	 /* this is an Internet address */
	serveraddr.sin_family = AF_INET;

	/* let the system figure out our IP address */
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* this is the port we will listen on */
	serveraddr.sin_port = htons((unsigned short)portno);

	/* 
	* bind: associate the parent socket with a port 
	*/
	if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	 sizeof(serveraddr)) < 0) 
	error("ERROR on binding");

	  /* 
	   * listen: make this socket ready to accept connection requests 
	  */
	if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
		 error("ERROR on listen");

	/* 
	* main loop: wait for a connection request, echo input line, 
	* then close connection.
	*/
	/* 
	* accept: wait for a connection request 
	*/
	childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
	if (childfd < 0) 
		error("ERROR on accept");
   
	/* 
	* gethostbyaddr: determine who sent the message 
	*/
	hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
	sizeof(clientaddr.sin_addr.s_addr), AF_INET);
	if (hostp == NULL)
		error("ERROR on gethostbyaddr");
	hostaddrp = inet_ntoa(clientaddr.sin_addr);
	if (hostaddrp == NULL)
		error("ERROR on inet_ntoa\n");
	printf("server established connection with %s (%s)\n", 
	hostp->h_name, hostaddrp);
	clientlen = sizeof(clientaddr);
	struct sllp_raw_packet request;
	struct sllp_raw_packet response;
	while (1) {
    
		/* 
		* read: read input string from the client
		*/
		bzero(buf, BUFSIZE);
		n = read(childfd, buf, BUFSIZE);
		if (n < 0) 
			error("ERROR reading from socket");
		printf("server received %d bytes: %s", n, buf);
    		
		request.data = buf;
		request.len = n;
		sllp_process_packet (sllp,&request,&response);
		/* 
		* write: echo the input string back to the client 
		*/
		n = write(childfd, response.data, response.len);
		if (n < 0) 
			error("ERROR writing to socket");
	
		close(childfd);
	}
	return 0;
}

