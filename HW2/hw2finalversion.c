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
#include "hw22.h"


//external reference to HandleEvents function
extern void HandleEvents(DNSServiceRef serviceRef, int listen_socket);

//timout value
#define LONG_TIME 100000000

//loop flag for an error and setting timeout value
static volatile int stopNow = 0;
static volatile int timeOut = LONG_TIME;
DNSServiceErrorType err;



//array containing file descriptors for 2 clients
int clients_fd[2];
//size of the name of oth players in bytes
int p1namesize;
int p2namesize;

//names of both players
char player1[128];
char player2[128];

//choices of each player
char choice1[10];
char choice2[10];

//flags indicating whther or not a client has input their name yet
int client1nameflag=0;
int client2nameflag=0;

//flas checking if clients have input their choices yet
int client1choiceflag=0;
int client2choiceflag=0;

//reserve integer  and buffer to check for disconnect after choices are locked in
int n;
char buffer[1];



//Function that checks whether or not a character array is a valid name of non-whitespace characters
//Parameters: 
//name: charachter array representing the name terminated by a newline and a null terminator
//size: length of the name to check not counting the null terminator
int isvalidname(char* name, int size)
{	
	int i;
	int space_count=0;
	for (i=0; i < size; i++)
	{	//if we find a whitespace character, add 1 to total
		if (isspace(name[i])) {space_count++;}
	}
	
	//if length of the name is equal to number of whitespace characters found, return false
	if (space_count == size) {return 0;}
	else {return 1;}
}


//Function that checks whether or not a character array is a valid choice in the set of rock paper scissors
//Parameters: 
//name: charachter array representing the choice terminated by a null terminator, all choices are assumed to have a newline as part of their iput as well
//size: length of the choice to check not counting the null terminator
int isvalidchoice(char* choice, int size)
{
	int i;
	for (i=0; i < size; i++)
	{
		choice[i] = tolower(choice[i]); //make the choice all lowercas eone character at a time to ignore case issues
	}

	//if choice isnt in our set return false
	if (strcmp(choice, "rock\n\0") != 0 && strcmp(choice, "paper\n\0") !=0 && strcmp(choice, "scissors\n\0") !=0 )  {return 0;}
	else {return 1;}

}

//Function that simulates the game of rock paper scissors to determine a winner
//PARAMETERS:
/*
s1: size of the first name excluding null termiantor and newline character
s2, size of the second name in bytes
c1: character array of player 1s choice
c2: player 2s choice
//sd1: file descriptor fo rht eifrst client
//sd2: file descriptor fo rth esecond client
*/
void game(int s1, int s2, char* c1, char*c2, int sd1, int sd2)
{

	int i=0;
	for (;;) //make player 1s name uppercase, break when we hit null terminator
	{
		if (player1[i] == '\0') break;
		player1[i]=toupper(player1[i]);
		i++;

	}

	i=0;  //make player 2s name uppercase, break until we hit null terminator
	for (;;)
	{
		if (player2[i] == '\0') break;
		player2[i]=toupper(player2[i]);
		i++;

	}


	//scenario whenboth players play rock: send tie to both clients
	if ((strcmp(c1, "rock\n")==0) && (strcmp(c2,"rock\n")==0))
	{
		send(sd1, "Tie!\n", sizeof("Tie!\n"), 0);
		send(sd2, "Tie!\n", sizeof("Tie!\n"), 0);
	} 

	//client 1 plays rock, clinet 2 plays paper, player 2 wins and gets their name snet first to each client
	else if ((strcmp(c1, "rock\n")==0) && (strcmp(c2,"paper\n")==0))
	{
		send(sd1, "PAPER covers ROCK! ", sizeof("PAPER covers ROCK! ")-1, 0);
		send(sd2, "PAPER covers ROCK! ", sizeof("PAPER covers ROCK! ")-1, 0);

		send(sd1, player2, s2, 0);
		send(sd2, player2, s2, 0);
		send(sd1, " defeats ", sizeof(" defeats ")-1,0); //sizeof in each of these examples ignores the null terminator upon sending
		send(sd2, " defeats ", sizeof(" defeats ")-1, 0);
		send(sd1, player1, s1, 0);
		send(sd2, player1, s1, 0);
		send(sd1, "!\n\0", 3,0);
		send(sd2, "!\n\0", 3,0);
	} 

	//client 1 plays rock, client 2 plays scissors, player 1 wins and gets their name sent first to each client
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

	//client 1 plays paper, clinet 2 plays rock, player 1 wins and gets their name sent first to each client
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

	//client 1 plays paper, clinet 2 plays paper, send tie to both clients
	else if ((strcmp(c1, "paper\n")==0) && (strcmp(c2,"paper\n")==0))
	{
		send(sd1, "Tie!\n", sizeof("Tie!\n"), 0);
		send(sd2, "Tie!\n", sizeof("Tie!\n"), 0);
	} 


	//client 1 plays paper, clinet 2 plays scissors, player 2 wins and gets their name sent first to each client
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

	//client 1 plays scissors, clinet 2 plays rock, player 2 wins and gets their name sent first to each client
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

	//client 1 plays scissors, clinet 2 plays paper, player 1 wins and gets their name sent first to each client
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

	//client 1 plays scissors, clinet 2 plays scissors, send tie to both clients
	else if ((strcmp(c1, "scissors\n")==0) && (strcmp(c2,"scissors\n")==0))
	{
		send(sd1, "Tie!\n", sizeof("Tie!\n"), 0);
		send(sd2, "Tie!\n", sizeof("Tie!\n"), 0);
	}	 


}




//Function that handles the main loop of the the server after registering with Zeroconf
//PArameters: 
//serviceRef: service reference to Zeroconf
//listen_socket: socket that the TCP server is bound to
void HandleEvents(DNSServiceRef serviceRef, int listen_socket)
{
	int dns_sd_fd = DNSServiceRefSockFD(serviceRef); //file descriptor for Zeroconf events
	fd_set readfds; //file descriptor set for select()
	struct timeval tv; //timeout struct

	//incoming client data struct and size
	struct sockaddr_in client; 
	socklen_t clientlength = sizeof(client);

	//by default, both file descriptors for clients are closed off
	clients_fd[0] = -1;
	clients_fd[1] = -1;
	
	// main server loop
	while (!stopNow)
	{
		
		
		FD_ZERO(&readfds); //zero out file descriptor set
		FD_SET(dns_sd_fd, &readfds); //add Zeroconf fd everytime
		FD_SET(listen_socket, &readfds); //ad listener fd to set ot check for new connections
		if (clients_fd[0] != -1) {FD_SET(clients_fd[0], &readfds);} //if clients ar eno longer zero, then we have valid client sthat we want to see data from
		if (clients_fd[1] != -1) {FD_SET(clients_fd[1], &readfds);}

		//set timeout values
		tv.tv_sec = timeOut;
		tv.tv_usec = 0;

		//select() for all file desciptors in the set
		int result = select(FD_SETSIZE, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		//we have actiity on at least one socket
		if (result > 0) 
		{

			//if the zeroconf fd has acitivty, process the result with corresponding error value
			if (FD_ISSET(dns_sd_fd, &readfds)) {
				err = DNSServiceProcessResult(serviceRef);
			}
			

			//if there is activity on the TCP socket bound to a port, try accepting new connections
			if (FD_ISSET(listen_socket, &readfds))
			{			

				//accept the connection and chekc which fd is available
				int newconnection = accept(listen_socket, (struct sockaddr *) &client, (socklen_t *) &clientlength);


				//first fd is open, mark the connection in our arraay and acknowledge client with question
				if (clients_fd[0]==-1)
				{
					clients_fd[0] = newconnection;
					send(clients_fd[0], "What is your name?\n", sizeof("What is your name?\n"), 0);
				}
				
				//second fd is open, mark the connection in our arraay and acknowledge client with question
				else if (clients_fd[1]==-1)
				{
					clients_fd[1] = newconnection;
					send(clients_fd[1], "What is your name?\n", sizeof("What is your name?\n"), 0);
				}


			}
	
			//if the first client has a nonsero descriptor and there is activity on it
			if (clients_fd[0] > 0 && FD_ISSET(clients_fd[0], &readfds))
			{	

				//client has not input their name yet, do that first
				if (client1nameflag==0)
				{
					int x=0; //variable to check and make sure this is the first time we sent the what is your name call
					int p1namebytes; //bytes to receive
					//goto where we redo it if we get an invalid name
					getname:
					if (x==0) {x++;} //first time, dont resend name question
					else {send(clients_fd[0], "What is your name?\n", sizeof("What is your name?\n"), 0);} //resend question all subsequent iterations if a valid name is not received
					p1namebytes = recv(clients_fd[0], player1, 128, 0); //receive data from client

					//client sent nothing or was closed, hndle disconnect
					if (p1namebytes == 0) 
					{
					
						close(clients_fd[0]); //close connection
						bzero(&player1, sizeof(player1)); //zero out all data for this client, reset all flags and state
						bzero(&choice1, sizeof(choice1));
						client1nameflag=0;
						client1choiceflag=0;
						p1namesize=0;
						clients_fd[0]=-1; //free array to accdpt new clients
						continue; 
					}

					//null terminate what we got
					player1[p1namebytes] = '\0';

					//size of name not counting newline
					p1namesize=p1namebytes-1;
					//check if valid name, mark flag if valid or loop back to goto
					if (!isvalidname(player1, p1namebytes)) goto getname;
					send(clients_fd[0], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
					client1nameflag=1;

				}	
				

				//client has to input choice
				else if (client1choiceflag==0)
				{
					int choice1bytes; //size of choice
					int x = 0; //variable to check and make sure this is the first time we sent the what is your choice message

					//loop back here if choice is bad
					get_choice:
					if (x==0) {x++;}
					else {send(clients_fd[0], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);}
					choice1bytes = recv(clients_fd[0], choice1, 10, 0); //receive choice form client

					//client sent 0 bytes or closed connection, handle disconnect cleanly
					if(choice1bytes == 0) 
					{	
						//close descriptor, free it up in array, and reset all flags and state data for client 1
						close(clients_fd[0]);
						bzero(&player1, sizeof(player1));
						bzero(&choice1, sizeof(choice1));
						client1nameflag=0;
						client1choiceflag=0;
						clients_fd[0]=-1;
						continue;
					}
					//null terminate choice and check what we got, set flag if valid or loop back if it is not
					choice1[choice1bytes] = '\0';

					if (!isvalidchoice(choice1, choice1bytes)) goto get_choice;
					client1choiceflag=1;
					
					
					
				}

				//client input both choices, but client still sent 0 bytes or closed connection, handle this disconnect cleanly
				else if ((n=recv(clients_fd[0], buffer, 1, 0)) == 0)
				{
					
					close(clients_fd[0]);
					bzero(&player1, sizeof(player1));
					bzero(&choice1, sizeof(choice1));
					client1nameflag=0;
					client1choiceflag=0;
					clients_fd[0]=-1;
					continue;


				}
			}

			//client 2 has a valid connection and there is activity on the fd
			if (clients_fd[1] > 0 && FD_ISSET(clients_fd[1], &readfds))
			{
				//clinet 2 has yet to enter a name
				if (client2nameflag==0)
				{
					int x=0;
					int p2namebytes; //sizeof name minus newline
					getname2:
					if (x==0) {x++;} //variables to check repeat sending of waht is name message like client 1
					else {send(clients_fd[1], "What is your name?\n", sizeof("What is your name?\n"), 0);}
					p2namebytes = recv(clients_fd[1], player2, 128, 0);

					//client closed connection or sent 0 bytes, make a clean disconnect
					if (p2namebytes == 0) 
					{
						//close fd for clien t2, reset state and flags, free up array for new connection
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
					//null temrinate what we got and check size of name
					//check ifname is valid or not and loop around f not valid
					if (!isvalidname(player2, p2namebytes)) goto getname2;
					send(clients_fd[1], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);
					client2nameflag=1;

				}	
				

				//cline thas ye tto make a choice for game
				else if (client2choiceflag==0)
				{
					int choice2bytes; //bytes received for choice 2
					int x = 0;
					//goto for invalid choice
					get_choice2: 
					if (x==0) {x++;} //logic for handling repeat sending o message so client doesnt see it twice after entering name
					else {send(clients_fd[1], "Rock, paper, or scissors?\n", sizeof("Rock, paper, or scissors?\n"), 0);}
					choice2bytes = recv(clients_fd[1], choice2, 10, 0);
					//check bytes received, 0 indicates client sent nothing or disconnected early
					if(choice2bytes == 0) 
					{	
						//close the client fd, free up fd in array and reset state of the client
						close(clients_fd[1]);
						bzero(&player2, sizeof(player2));
						bzero(&choice2, sizeof(choice2));
						client2nameflag=0;
						client2choiceflag=0;
						clients_fd[1]=-1;
						continue;
					}
					
					choice2[choice2bytes] = '\0'; //check if we got a valid choice and set the flag if we did
					if (!isvalidchoice(choice2, choice2bytes)) goto get_choice2;
					client2choiceflag=1;
					
				}

				//client has entered both choices but disconnected or sent 0 bytes afterward, close connection
				else if ((n=recv(clients_fd[1], buffer, 1, 0)) == 0)
				{
					//close the client fd, free up fd in array and reset state of the client
					close(clients_fd[1]);
					bzero(&player2, sizeof(player2));
					bzero(&choice2, sizeof(choice2));
					client2nameflag=0;
					client2choiceflag=0;
					clients_fd[1]=-1;
					continue;


				}
								
				
			
			}

			//if all choices have been entered, then play the game

			if (client1choiceflag==1 && client2choiceflag==1 && client1nameflag==1 && client2nameflag==1) 
			{
				game(p1namesize, p2namesize, choice1, choice2, clients_fd[0], clients_fd[1]);

				//once game is over, reset state for both clinets and free up their descriptors for two new players
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


			if (err) {stopNow = 1;} //error, stop server
		}

	}

}

//callback funtion for DNS, indicated successful registration, parameters are outlined in textbook
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


//function that registers the client with DNS
//PARAMETERS: 
//tport: port number we are binding to zeroconf
//lisnter_socket: TCP socket bound to our local port
static DNSServiceErrorType MyDNSServiceRegister(int tport, int listener_socket)
{
	DNSServiceErrorType error;
	DNSServiceRef serviceRef;
	
	error = DNSServiceRegister(&serviceRef,
								0,                  // no flags
								0,                  // all network interfaces
								"mohrm",  		// name
								"_rps._tcp",       // service type
								"local",                 // register in default domain(s)
								NULL,               // use default host name
								htons(tport),        // port number
								0,                  // length of TXT record
								NULL,               // no TXT record
								MyRegisterCallBack, // call back function
								NULL);              // no context
	
	if (error == kDNSServiceErr_NoError)
	{
		HandleEvents(serviceRef, listener_socket); //no error, then handle event with TCP socket
		DNSServiceRefDeallocate(serviceRef);
	}
	
	return error;
}






//main funciton to set up server
int main()
{
	//define listener socket
	int socket_listen = socket(PF_INET, SOCK_STREAM, 0);
	if (socket_listen==-1) {perror("ERROR: SOCKET FAILED\n"); return EXIT_FAILURE;}

	//TCP server struct
	struct sockaddr_in tserver;
	tserver.sin_family = AF_INET;
	tserver.sin_addr.s_addr = INADDR_ANY;
	 
	//bind to random OS assigned port
	tserver.sin_port = htons(0);
	int len = sizeof(tserver);

	if (bind(socket_listen, (struct sockaddr *) &tserver, len) == -1)
	{
		perror("ERROR: bind() failed\n"); 
		return EXIT_FAILURE;
	}


	//inndicate we are now listening, indicating TCP socket
	if (listen(socket_listen,5) == -1)
	{
		perror("ERROR: listen() failed\n"); 
		return EXIT_FAILURE;
	}

	//get socket name information
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

