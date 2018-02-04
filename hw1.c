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


  struct sockaddr_in client2;
  int len2 = sizeof( client2 );

  char buffer[500];
  int k = recvfrom( sd, buffer, 500, 0, (struct sockaddr *) &client2,
                  (socklen_t *) &len2 );

  printf("%d\n", k);

  short opcode = buffer[0] + buffer[1];


  printf("%d\n", opcode);

  int i;
  for (i=2; i < k; i++)
  {
    if (buffer[i] == '\0')
    {
	    printf("\nnull terminator\n");
	    continue;
    }
    printf("%c", buffer[i]);
  }




  printf("\n");
  return 0;

}
