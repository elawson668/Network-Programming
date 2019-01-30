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
#include <ctype.h>

#define LONG_TIME 100000000

static volatile int stopNow = 0;
static volatile int timeOut = LONG_TIME;
DNSServiceErrorType err;

int clients_fd[2];
int p1namesize;
int p2namesize;
char player1[128];
char player2[128];
char choice1[10];
char choice2[10];
int client1nameflag=0;
int client2nameflag=0;
int client1choiceflag=0;
int client2choiceflag=0;
int n;
char buffer[1];




int isvalidname(char* name, int size)
{	int i;
	int space_count=0;
	for (i=0; i < size; i++)
	{
		if (isspace(name[i])) {space_count++;}
	}
	
	if (space_count == size) {return 0;}
	else {return 1;}




}

int isvalidchoice(char* choice, int size)
{
	int i;
	for (i=0; i < size; i++)
	{
		choice[i] = tolower(choice[i]);
	}

	
	if (strcmp(choice, "rock\n\0") != 0 && strcmp(choice, "paper\n\0") !=0 && strcmp(choice, "scissors\n\0") !=0 )  {return 0;}
	else {return 1;}

}


void game(int s1, int s2, char* c1, char*c2, int sd1, int sd2)

{

int i=0;
for (;;)
{
if (player1[i] == '\0') break;
player1[i]=toupper(player1[i]);
i++;

}

i=0;
for (;;)
{
if (player2[i] == '\0') break;
player2[i]=toupper(player2[i]);
i++;

}

if ((strcmp(c1, "rock\n")==0) && (strcmp(c2,"rock\n")==0))
{
send(sd1, "Tie!\n", sizeof("Tie!\n"), 0);
send(sd2, "Tie!\n", sizeof("Tie!\n"), 0);

} 

else if ((strcmp(c1, "rock\n")==0) && (strcmp(c2,"paper\n")==0))
{

send(sd1, "PAPER covers ROCK! ", sizeof("PAPER covers ROCK! ")-1, 0);
send(sd2, "PAPER covers ROCK! ", sizeof("PAPER covers ROCK! ")-1, 0);

send(sd1, player2, s2, 0);
send(sd2, player2, s2, 0);
send(sd1, " defeats ", sizeof(" defeats ")-1,0);
send(sd2, " defeats ", sizeof(" defeats ")-1, 0);
send(sd1, player1, s1, 0);
send(sd2, player1, s1, 0);
send(sd1, "!\n\0", 3,0);
send(sd2, "!\n\0", 3,0);

} 


else if ((strcmp(c1, "rock\n")==0) && (strcmp(c2,"scissors\n")==0))
{

send(sd1, "ROCK smashes SCISSORS! ", sizeof("ROCK smashes SCISSORS! ")-1, 0);
send(sd2, "ROCK smashes SCISSORS! ", sizeof("ROCK smashes SCISSORS! ")-1, 0);

send(sd1, player1, s1, 0);
send(sd2, player1, s1, 0);
send(sd1, " defeats ", sizeof(" defeats ")-1,0);
send(sd2, " defeats ", sizeof(" defeats ")-1, 0);
send(sd1, player2, s2, 0);
send(sd2, player2, s2, 0);
send(sd1, "!\n\0", 3,0);
send(sd2, "!\n\0", 3,0);

} 


else if ((strcmp(c1, "paper\n")==0) && (strcmp(c2,"rock\n")==0))
{

send(sd1, "PAPER covers ROCK! ", sizeof("PAPER covers ROCK! ")-1, 0);
send(sd2, "PAPER covers ROCK! ", sizeof("PAPER covers ROCK! ")-1, 0);

send(sd1, player1, s1, 0);
send(sd2, player1, s1, 0);
send(sd1, " defeats ", sizeof(" defeats ")-1,0);
send(sd2, " defeats ", sizeof(" defeats ")-1, 0);
send(sd1, player2, s2, 0);
send(sd2, player2, s2, 0);
send(sd1, "!\n\0", 3,0);
send(sd2, "!\n\0", 3,0);

} 

else if ((strcmp(c1, "paper\n")==0) && (strcmp(c2,"paper\n")==0))
{

send(sd1, "Tie!\n", sizeof("Tie!\n"), 0);
send(sd2, "Tie!\n", sizeof("Tie!\n"), 0);

} 

else if ((strcmp(c1, "paper\n")==0) && (strcmp(c2,"scissors\n")==0))
{

send(sd1, "SCISSORS cuts PAPER! ", sizeof("SCISSORS cuts PAPER! ")-1, 0);
send(sd2, "SCISSORS cuts PAPER! ", sizeof("SCISSORS cuts PAPER! ")-1, 0);

send(sd1, player2, s2, 0);
send(sd2, player2, s2, 0);
send(sd1, " defeats ", sizeof(" defeats ")-1,0);
send(sd2, " defeats ", sizeof(" defeats ")-1, 0);
send(sd1, player1, s1, 0);
send(sd2, player1, s1, 0);
send(sd1, "!\n\0", 3,0);
send(sd2, "!\n\0", 3,0);

}
else if ((strcmp(c1, "scissors\n")==0) && (strcmp(c2,"rock\n")==0))
{

send(sd1, "ROCK smashes SCISSORS! ", sizeof("ROCK smashes SCISSORS! ")-1, 0);
send(sd2, "ROCK smashes SCISSORS! ", sizeof("ROCK smashes SCISSORS! ")-1, 0);

send(sd1, player2, s2, 0);
send(sd2, player2, s2, 0);
send(sd1, " defeats ", sizeof(" defeats ")-1,0);
send(sd2, " defeats ", sizeof(" defeats ")-1, 0);
send(sd1, player1, s1, 0);
send(sd2, player1, s1, 0);
send(sd1, "!\n\0", 3,0);
send(sd2, "!\n\0", 3,0);
}


else if ((strcmp(c1, "scissors\n")==0) && (strcmp(c2,"paper\n")==0))
{
send(sd1, "SCISSORS cuts PAPER! ", sizeof("SCISSORS cuts PAPER! ")-1, 0);
send(sd2, "SCISSORS cuts PAPER! ", sizeof("SCISSORS cuts PAPER! ")-1, 0);

send(sd1, player1, s1, 0);
send(sd2, player1, s1, 0);
send(sd1, " defeats ", sizeof(" defeats ")-1,0);
send(sd2, " defeats ", sizeof(" defeats ")-1, 0);
send(sd1, player2, s2, 0);
send(sd2, player2, s2, 0);
send(sd1, "!\n\0", 3,0);
send(sd2, "!\n\0", 3,0);

}

else if ((strcmp(c1, "scissors\n")==0) && (strcmp(c2,"scissors\n")==0))
{
send(sd1, "Tie!\n", sizeof("Tie!\n"), 0);
send(sd2, "Tie!\n", sizeof("Tie!\n"), 0);

} 




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
	socklen_t clientlength = sizeof(client);
	clients_fd[0] = -1;
	clients_fd[1] = -1;
	
	
	// . . .
	while (!stopNow)
	{
		
		
		FD_ZERO(&readfds);
		FD_SET(dns_sd_fd, &readfds);
		FD_SET(listen_socket, &readfds);
		if (clients_fd[0] != -1) {FD_SET(clients_fd[0], &readfds);}
		if (clients_fd[1] != -1) {FD_SET(clients_fd[1], &readfds);}
		tv.tv_sec = timeOut;
		tv.tv_usec = 0;

		int result = select(FD_SETSIZE, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		if (result ==0) {continue;}
		
			
			if (FD_ISSET(dns_sd_fd, &readfds))
				err = DNSServiceProcessResult(serviceRef);
			
			if (FD_ISSET(listen_socket, &readfds))
			{			

				int newconnection = accept(listen_socket, (struct sockaddr *) &client, &clientlength);
				if (clients_fd[0]==-1)
				{
				
				clients_fd[0] = newconnection;
				send(clients_fd[0], "What is your name?\n", sizeof("What is your name?\n"), 0);
				
				
				}
				
				else if (clients_fd[1]==-1)
				{
				
				clients_fd[1] = newconnection;
				send(clients_fd[1], "What is your name?\n", sizeof("What is your name?\n"), 0);
				}


			}
	
			if (FD_ISSET(clients_fd[0], &readfds))
			{	
				if (client1nameflag==0)
				{
					int x=0;
					int p1namebytes;
					getname:
					if (x==0) {x++;}
					else {send(clients_fd[0], "What is your name?\n", sizeof("What is your name?\n"), 0);}
					p1namebytes = recv(clients_fd[0], player1, 128, 0);
					if (p1namebytes == 0) 
					{
					
						printf("EARLY DISCONNECT\n"); 
						close(clients_fd[0]);
						bzero(&player1, sizeof(player1));
						bzero(&choice1, sizeof(choice1));
						client1nameflag=0;
						client1choiceflag=0;
						p1namesize=0;
						clients_fd[0]=-1;
						continue;
					}
					player1[p1namebytes] = '\0';
					p1namesize=p1namebytes-1;
					if (!isvalidname(player1, p1namebytes)) goto getname;
					send(clients_fd[0], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
					client1nameflag=1;

				}	
				
				else if (client1choiceflag==0)
				{
					int choice1bytes;
					int x = 0;
					get_choice:
					if (x==0) {x++;}
					else {send(clients_fd[0], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);}
					choice1bytes = recv(clients_fd[0], choice1, 10, 0);
					if(choice1bytes == 0) 
					{	
						printf("EARLY DISCONNECT\n"); 
						close(clients_fd[0]);
						bzero(&player1, sizeof(player1));
						bzero(&choice1, sizeof(choice1));
						client1nameflag=0;
						client1choiceflag=0;
						clients_fd[0]=-1;
						continue;
					}
					
					choice1[choice1bytes] = '\0';
					if (!isvalidchoice(choice1, choice1bytes)) goto get_choice;
					client1choiceflag=1;
					printf("DONE\n");
					
					
				}

				else if ((n=recv(clients_fd[0], buffer, 1, 0)) == 0)
				{
					printf("EARLY DISCONNECT\n"); 
					close(clients_fd[0]);
					bzero(&player1, sizeof(player1));
					bzero(&choice1, sizeof(choice1));
					client1nameflag=0;
					client1choiceflag=0;
					clients_fd[0]=-1;
					continue;


				}
			}


			if (FD_ISSET(clients_fd[1], &readfds))
			{
				if (client2nameflag==0)
				{
					int x=0;
					int p2namebytes;
					getname2:
					if (x==0) {x++;}
					else {send(clients_fd[1], "What is your name?\n", sizeof("What is your name?\n"), 0);}
					p2namebytes = recv(clients_fd[1], player2, 128, 0);
					if (p2namebytes == 0) 
					{
					
						printf("EARLY DISCONNECT\n"); 
						close(clients_fd[1]);
						bzero(&player2, sizeof(player2));
						bzero(&choice2, sizeof(choice2));
						client2nameflag=0;
						client2choiceflag=0;
						p2namesize=0;
						clients_fd[1]=-1;
						continue;
					}
					player2[p2namebytes] = '\0';
					p2namesize=p2namebytes-1;
					if (!isvalidname(player2, p2namebytes)) goto getname2;
					send(clients_fd[1], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
					client2nameflag=1;

				}	
				
				else if (client2choiceflag==0)
				{
					int choice2bytes;
					int x = 0;
					get_choice2:
					if (x==0) {x++;}
					else {send(clients_fd[1], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);}
					choice2bytes = recv(clients_fd[1], choice2, 10, 0);
					if(choice2bytes == 0) 
					{	
						printf("EARLY DISCONNECT\n"); 
						close(clients_fd[1]);
						bzero(&player2, sizeof(player2));
						bzero(&choice2, sizeof(choice2));
						client2nameflag=0;
						client2choiceflag=0;
						clients_fd[1]=-1;
						continue;
					}
					
					choice2[choice2bytes] = '\0';
					if (!isvalidchoice(choice2, choice2bytes)) goto get_choice2;
					client2choiceflag=1;
					
				}

				else if ((n=recv(clients_fd[1], buffer, 1, 0)) == 0)
				{
					printf("EARLY DISCONNECT\n"); 
					close(clients_fd[1]);
					bzero(&player2, sizeof(player2));
					bzero(&choice2, sizeof(choice2));
					client2nameflag=0;
					client2choiceflag=0;
					clients_fd[1]=-1;
					continue;


				}
								
				
			
			}

			if (client1choiceflag==1 && client2choiceflag==1 && client1nameflag==1 & client2nameflag==1) 
			{
			game(p1namesize, p2namesize, choice1, choice2, clients_fd[0], clients_fd[1]);

			close(clients_fd[0]);
			bzero(&player1, sizeof(player1));
			bzero(&choice1, sizeof(choice1));
			client1nameflag=0;
			client1choiceflag=0;
			p1namesize=0;
			clients_fd[0]=-1;

			close(clients_fd[1]);
			bzero(&player2, sizeof(player2));
			bzero(&choice2, sizeof(choice2));
			client2nameflag=0;
			client2choiceflag=0;
			p2namesize=0;
			clients_fd[1]=-1;
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







int main()
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

	if (listen(socket_listen,5) == -1)
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
