#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>

<<<<<<< HEAD
=======
typedef union 
{

  uint16_t opcode;

  struct 
  {
    uint16_t opcode;
    uint16_t data[514];
  } request;

  struct 
  {
    uint16_t opcode;
    uint16_t block_num;
    uint16_t data[512];
  } data;

  struct
  {
    uint16_t opcode;
    uint16_t block_num;
  } ack;

  struct 
  {
    uint16_t opcode;
    uint16_t err_code;
    uint16_t message[512];
  } error;

} PACKET;





>>>>>>> 194d26f59b46b45da07500875c9afe4fd2a73057

int main (int argc, char* argv[])
{

int sd; 
struct sockaddr_in udpserver;
int length = sizeof(udpserver);

sd = socket( AF_INET, SOCK_DGRAM, 0 ); 

  if ( sd < 0 )  
  {
    perror( "socket() failed" );
    return EXIT_FAILURE;
  }

bzero(&udpserver, length);
udpserver.sin_family = AF_INET;
udpserver.sin_addr.s_addr = htonl(INADDR_ANY);
udpserver.sin_port = htons(0); 

if ( bind( sd, (struct sockaddr *) &udpserver, length ) < 0 )
  {
    perror( "bind() failed" );
    return EXIT_FAILURE;
  }


  if ( getsockname( sd, (struct sockaddr *) &udpserver, (socklen_t *) &length ) < 0 )
  {
    perror( "getsockname() failed" );
    return EXIT_FAILURE;
  }

  printf( "TFTP server assigned to port number %d\n", ntohs( udpserver.sin_port ) );


 struct sockaddr_in client;
  int leng = sizeof( client );

int pid;
while (1)
{

	char buffer[1024];
	int bytes_read = recvfrom( sd, buffer, 1024, 0, (struct sockaddr *) &client,
                  (socklen_t *) &leng );

	
	if (bytes_read > 0)
	{

	pid = fork();
	
	if (pid ==0)
	{
		printf("%d\n", bytes_read);

		uint16_t opcode = buffer[0] + buffer[1];

		printf("%d\n", opcode);

		int i;
		for (i=0; i < bytes_read; i++)
		{
			if (buffer[i] == '\0')
			{
			printf("\nnull terminator\n");
			continue;
			}
		
		printf("%c", buffer[i]);
		}
		printf("\n");
	}

	}
	

}





return 0;

}
