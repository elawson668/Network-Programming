
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/select.h>  
#include <cstring>
#include <string>
#include <map>  
#include <vector>
#include <iostream>


using namespace std;


#define LONG_TIME 100000000

int client_fds[100];
char buffer[1024];    

map<string,vector<string> > channels;
map<int,string> users;
map<int,bool> operators;


static volatile int timeOut = LONG_TIME;

int main()
{
	
    int listener_socket = socket( AF_INET, SOCK_STREAM, 0 );

	if ( listener_socket < 0 )
	{
	  perror( "socket()" );
	  exit( EXIT_FAILURE );
	}


	struct sockaddr_in server;
	struct sockaddr_in client;

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(0);
	int len = sizeof( server );

	if ( bind( listener_socket, (struct sockaddr *)&server, len ) < 0 )
	{
	  perror( "bind()" );
	  exit( EXIT_FAILURE );
	}

	if (getsockname (listener_socket, (struct sockaddr *) &server, (socklen_t *) &len) < 0)
	{
		perror("ERROR: getsockname() failed\n"); 
		return EXIT_FAILURE;
	}

	
	
	int tport = ntohs(server.sin_port);
	printf("%d\n", tport);
	listen( listener_socket, 5 ); 
	int clientlen = sizeof( client );
	struct timeval tv;
	tv.tv_sec = timeOut;
	tv.tv_usec = 0;


	int index=0;
	while ( 1 )
  	{
		fd_set readfds;
		FD_ZERO( &readfds );
    	FD_SET( listener_socket, &readfds );
		for (int i = 0 ; i < index ; i++ )
    	{
      		FD_SET( client_fds[ i ], &readfds );
        }

		int result = select(FD_SETSIZE, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
		if (result ==0) {continue;}
		if (FD_ISSET(listener_socket, &readfds))
		{			

			int newconnection = accept(listener_socket, (struct sockaddr *) &client, (socklen_t *) &clientlen);
			client_fds[ index++ ] = newconnection;
		}

		for ( int j = 0 ; j < index ; j++ )
		{
			int fd = client_fds[ j ];
		    if ( FD_ISSET( fd, &readfds ) )
			{
				int incoming_bytes=recv(fd, buffer, 1024, 0);
				
				if (incoming_bytes == 0)
				{
					close( fd );
					for ( int a = 0 ; a < index ; a++ ) //find the descriptor that is closing
					{
						if ( fd == client_fds[ a ] ) //we found it, copy everything that connect after it
						{
						     
							for ( int b = a ; b < index - 1 ; b++ ) //go up to the last index we have -1, we are copying the i+1th position to 													postion i
						    {
								client_fds[ b ] = client_fds[ b + 1 ];
						    }
						    index--;
						    break;     
					   	}
					}	

				}

				else
				{

					printf("CLIENT %d sent me data\n", fd); 

					string buf = buffer;

					if(buf.substr(0,4).compare("USER") == 0) {

						string name = buf.substr(5);
						if(name.length() > 20) {
							send(fd, "Invalid username\n", sizeof("Invalid username\n"),0);
							bzero(buffer,1024);
							continue;
						}

						users.insert(pair<int,string>(fd,name));
						operators.insert(pair<int,bool>(fd,false));
					}



				}

			}
	
  
		}


	}



}
