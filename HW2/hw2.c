// Skeleton of how to handle DNSServiceDiscovery events using a select() loop

#include "dns_sd.h"
#include <stdio.h>
#include <stdlib.h>			// For stdout, stderr
#include <string.h>			// For strlen(), strcpy(), bzero()
#include <errno.h>          // For errno, EINTR
#include <time.h>
#include <sys/time.h>		// For struct timeval
#include <unistd.h>         // For getopt() and optind
#include <arpa/inet.h>		// For inet_addr()
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>

#define LONG_TIME 100000000

static volatile int stopNow = 0;
static volatile int timeOut = LONG_TIME;
DNSServiceErrorType err;

void HandleEvents(DNSServiceRef serviceRef, int listen_socket)
	{
	int dns_sd_fd = DNSServiceRefSockFD(serviceRef);
	int nfds = dns_sd_fd + 1;
	int max_sd;
	if (nfds > listen_socket+1)
		max_sd = nfds;
	else
		max_sd = listen_socket+1;
	fd_set readfds;
	struct timeval tv;
	struct sockaddr_in client;
	int clientlength = sizeof(client);
	int clients_fd[2];
	clients_fd[0] = -1;
	clients_fd[1] = -1;
	char player1[128];
	char player2[128];
	char choice1[8];
	char choice2[8];
	
	
	// . . .
	while (!stopNow)
		{
		
		
		FD_ZERO(&readfds);
		FD_SET(dns_sd_fd, &readfds);
		FD_SET(listen_socket, &readfds);
		tv.tv_sec = timeOut;
		tv.tv_usec = 0;

		int result = select(max_sd, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		
		if (result > 0)
			{
			
			if (FD_ISSET(dns_sd_fd, &readfds))
				err = DNSServiceProcessResult(serviceRef);
			
			if (FD_ISSET(listen_socket, &readfds))
			{				
				int newconnection = accept(listen_socket, (struct sockaddr *) &client, &clientlength);
				printf("ACCEPTED\n");
				if (clients_fd[0] == -1)
					{
					clients_fd[0] = newconnection; 
					send(newconnection, "CLIENT 0 SET\n", sizeof("CLIENT 0 SET\n"), 0);
					send(newconnection, "What is your name?\n", sizeof("What is your name?\n"), 0);
					int p1namebytes = recv(newconnection, player1, 128, 0);
					player1[p1namebytes] = '\0';
					send(newconnection, "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
					int choice1bytes = recv(newconnection, choice1, 9, 0);
					choice1[choice1bytes] = '\0';
					
					}

				else if (clients_fd[1] == -1)
					{clients_fd[1] = newconnection; send(newconnection, "CLIENT 1 SET\n", sizeof("CLIENT 1 SET\n"), 0);}

				
				else
					send(newconnection,"MAX NUMBER OF CONNECTIONS MADE: DISCONNECTING", sizeof("MAX NUMBER OF CONNECTIONS MADE: DISCONNECTING"), 0);
					close(newconnection);
					continue;






			}
			if (err) stopNow = 1;
			

		


			}




		
		}


		
	}


static void
MyRegisterCallBack(DNSServiceRef service,
				   DNSServiceFlags flags,
				   DNSServiceErrorType errorCode,
				   const char * name,
				   const char * type,
				   const char * domain,
				   void * context)
	{
	#pragma unused(flags)
	#pragma unused(context)

	if (errorCode != kDNSServiceErr_NoError)
		fprintf(stderr, "MyRegisterCallBack returned %d\n", errorCode);
	else
		printf("%-15s %s.%s%s\n","REGISTER", name, type, domain);
	}

static DNSServiceErrorType MyDNSServiceRegister(tport, listener_socket)
	{
	DNSServiceErrorType error;
	DNSServiceRef serviceRef;
	
	error = DNSServiceRegister(&serviceRef,
								0,                  // no flags
								0,                  // all network interfaces
								"mohrm",  		// name
								"_rps._tcp",       // service type
								"",                 // register in default domain(s)
								NULL,               // use default host name
								htons(tport),        // port number
								0,                  // length of TXT record
								NULL,               // no TXT record
								MyRegisterCallBack, // call back function
								NULL);              // no context
	
	if (error == kDNSServiceErr_NoError)
		{
		HandleEvents(serviceRef, listener_socket);
		DNSServiceRefDeallocate(serviceRef);
		}
	
	return error;
	}







main()
	{


	int socket_listen = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_listen==-1) {perror("ERROR: SOCKET FAILED\n"); return EXIT_FAILURE;}


	struct sockaddr_in tserver;
	tserver.sin_family = AF_INET;
	tserver.sin_addr.s_addr = INADDR_ANY;
	 

	tserver.sin_port = htons(0);
	int len = sizeof(tserver);


	if (bind(socket_listen, (struct sockaddr *) &tserver, len) == -1)
	{
	perror("ERROR: bind() failed\n"); 
	return EXIT_FAILURE;
	}

	if (listen(socket_listen,2) == -1)
	{
	perror("ERROR: listen() failed\n"); 
	return EXIT_FAILURE;
	}

	if (getsockname (socket_listen, (struct sockaddr *) &tserver, (socklen_t *) &len) < 0)
	{
	perror("ERROR: getsockname() failed\n"); 
	return EXIT_FAILURE;
	}
	
	int tport = ntohs(tserver.sin_port);
	printf("Started server\n");
	fflush(stdout);
	printf("Listening for TCP connections on port: %u\n", tport);
	fflush(stdout);





	DNSServiceErrorType error = MyDNSServiceRegister(tport, socket_listen);
	fprintf(stderr, "DNSServiceDiscovery returned %d\n", error);
	return 0;
	}
