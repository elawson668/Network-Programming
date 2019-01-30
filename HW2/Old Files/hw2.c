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
#include <pthread.h>

#define LONG_TIME 100000000

static volatile int stopNow = 0;
static volatile int timeOut = LONG_TIME;
DNSServiceErrorType err;
pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2=PTHREAD_MUTEX_INITIALIZER;

int clients_fd[2];
char player1[128];
char player2[128];
char choice1[8];
char choice2[8];
int client1flag=0;
int client2flag=0;


/*
	else if (sd2 == -1)
	{	
		pthread_mutex_lock(&mutex);
		clients_fd[1] = connection_sd; 
		pthread_mutex_unlock(&mutex);

		send(connection_sd, "CLIENT 1 SET\n", sizeof("CLIENT 1 SET\n"), 0);
		send(connection_sd, "What is your name?\n", sizeof("What is your name?\n"), 0);
		int p2namebytes = recv(connection_sd, player2, 128, 0);
		player2[p2namebytes] = '\0';
		send(connection_sd, "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
		int choice2bytes = recv(connection_sd, choice2, 9, 0);
		choice2[choice2bytes] = '\0';
		free(arg);

	}

				
	else
	{
		send(connection_sd,"MAX NUMBER OF CONNECTIONS MADE: DISCONNECTING\n", sizeof("MAX NUMBER OF CONNECTIONS MADE: DISCONNECTING\n"), 0);
		close(connection_sd);
		free(arg);
		

	}	
*/



void* client_handler1( void* arg)
{

	int connection_sd=*(int*)arg;


	pthread_mutex_lock(&mutex);
	clients_fd[0] = connection_sd; 
	pthread_mutex_unlock(&mutex);



	send(connection_sd, "CLIENT 0 SET\n", sizeof("CLIENT 0 SET\n"), 0);
	send(connection_sd, "What is your name?\n", sizeof("What is your name?\n"), 0);
	int p1namebytes = recv(connection_sd, player1, 128, 0);
	if (p1namebytes == 0) 
	{
		pthread_mutex_lock(&mutex);
		printf("EARLY DISCONNECT\n"); 
		clients_fd[0]=-1;
		pthread_mutex_unlock(&mutex);
		return;
	}
	player1[p1namebytes] = '\0';
	send(connection_sd, "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
	int choice1bytes = recv(connection_sd, choice1, 9, 0);
	if(choice1bytes == 0) 
	{	
		pthread_mutex_lock(&mutex);
		printf("EARLY DISCONNECT\n"); 
		clients_fd[0]=-1;
		pthread_mutex_unlock(&mutex);
		return;
	}
	choice1[choice1bytes] = '\0';

	pthread_mutex_lock(&mutex);
	client1flag = 1;
	pthread_mutex_unlock(&mutex);

		



	free(arg);
	

		
	


}






void* client_handler2( void* arg)
{
	
		int connection_sd=*(int*)arg;
		pthread_mutex_lock(&mutex);
		clients_fd[1] = connection_sd; 
		pthread_mutex_unlock(&mutex);

		send(connection_sd, "CLIENT 1 SET\n", sizeof("CLIENT 1 SET\n"), 0);
		send(connection_sd, "What is your name?\n", sizeof("What is your name?\n"), 0);
		int p2namebytes = recv(connection_sd, player2, 128, 0);
		if (p2namebytes == 0) 

		{
			pthread_mutex_lock(&mutex);
			printf("EARLY DISCONNECT\n"); 
			clients_fd[1]=-1;
			pthread_mutex_unlock(&mutex);
			return;
		}

		player2[p2namebytes] = '\0';
		send(connection_sd, "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
		int choice2bytes = recv(connection_sd, choice2, 9, 0);
		if(choice2bytes == 0) 
		{
			pthread_mutex_lock(&mutex);
			printf("EARLY DISCONNECT\n"); 
			clients_fd[1]=-1;
			pthread_mutex_unlock(&mutex);
			return;
		}
		choice2[choice2bytes] = '\0';

		pthread_mutex_lock(&mutex);
		client2flag = 1;
		pthread_mutex_unlock(&mutex);

		
		while(1)
		{
			pthread_mutex_lock(&mutex);
			if (client1flag == 1 && client2flag==1)
			{
			send(clients_fd[0], "Done\n", sizeof("Done\n"), 0);
			send(clients_fd[1], "Done\n", sizeof("Done\n"), 0);
			close(clients_fd[0]);
			close(clients_fd[1]);

			clients_fd[0]=-1;
			clients_fd[1]=-1;
			client1flag=0;
			client2flag=0;

			}
			pthread_mutex_unlock(&mutex);
		}

		free(arg);


}





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
	clients_fd[0] = -1;
	clients_fd[1] = -1;
	
	
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

				pthread_mutex_lock(&mutex);
				int sd1 = clients_fd[0];
				int sd2 = clients_fd[1];
				int status1 = client1flag;
				int status2 = client2flag;
				pthread_mutex_unlock(&mutex);

				if (sd1==-1 && status1 !=1 )
				{
					int newconnection = accept(listen_socket, (struct sockaddr *) &client, &clientlength);
					printf("ACCEPTED\n");
					pthread_t tid;
					int * argd = (int*) malloc(sizeof(int));
					*argd=newconnection;
					pthread_create(&tid, NULL, client_handler1,  (void *) argd);
				}


				else if (sd2==-1 && status2 != 1)
				{
					int newconnection = accept(listen_socket, (struct sockaddr *) &client, &clientlength);
					printf("ACCEPTED\n");
					pthread_t tid;
					int * argd = (int*) malloc(sizeof(int));
					*argd=newconnection;
					pthread_create(&tid, NULL, client_handler2,  (void *) argd);
				}
			
				
			}
			if (err) {stopNow = 1;}
			
			
			
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
