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
#include <string>
#include "hw1.h"
#include <iostream>
#include <signal.h>
using namespace std;

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

int timeouts = 0;

void handle_alarm(int signum) {

	timeouts++;
	if(timeouts == 10) {
		kill(getpid(),SIGTERM);
	}

}

void handle_error(int sd, struct sockaddr * client, socklen_t* length, uint16_t errcode, string message) {

	char packet[4+sizeof(message)];
	char* pack_ptr = packet;
    uint16_t eopcode = ERROR;
	eopcode = htons(eopcode);
	errcode = htons(errcode);
	memcpy(pack_ptr, &eopcode, sizeof(uint16_t));
	pack_ptr += sizeof(uint16_t);
	memcpy(pack_ptr, &errcode, sizeof(uint16_t));
	pack_ptr+=sizeof(uint16_t);
	memcpy(pack_ptr, &message, sizeof(message));
	sendto(sd, packet, sizeof(packet),0, (struct sockaddr *) client, *length);

}

void handle_read_request(char* filename, struct sockaddr * client, socklen_t* length)
{	
	
	int sdchild = socket( AF_INET, SOCK_DGRAM, 0 ) ; 
	struct sockaddr_in serverchild;
	int length_child = sizeof(serverchild);

	if ( sdchild < 0 )  
	{
		perror( "socket() failed" );
    		
	}

	bzero(&serverchild, length_child);
	serverchild.sin_family = AF_INET;
	serverchild.sin_addr.s_addr = htonl(INADDR_ANY);
	serverchild.sin_port = htons(0); 

	if ( bind( sdchild, (struct sockaddr *) &serverchild, length_child ) < 0 )
 	{
   		perror( "bind() failed" );
   		
  	}

	FILE* file_to_read = fopen(filename, "r");
	if (file_to_read == NULL) 
	{	

		uint16_t errcode= 1;
		string message = "File Not Found";
		handle_error(sdchild,client,length,errcode,message);
					
	}

	
	else
	{

		char read_buf[512];
		bzero(read_buf, 512);
		uint16_t block=1;
		while (true)
		{
			int bytes_read = fread(read_buf, 1, 512, file_to_read);
			printf("%d bytes read from file\n", bytes_read);

			if (bytes_read < 512)
			{
				printf("%d bytes left. This is last packet\n", bytes_read);
				char last_pack[bytes_read+4];
				bzero(last_pack, bytes_read);
				char* last_ptr = last_pack;
				uint16_t opcode = DATA;
				opcode=htons(opcode);
				block=htons(block);
				memcpy(last_ptr, &opcode, sizeof(uint16_t));
				last_ptr+=sizeof(uint16_t);
				memcpy(last_ptr, &block, sizeof(uint16_t));
				last_ptr+=sizeof(uint16_t);
				memcpy(last_ptr, &read_buf, bytes_read);
				sendto(sdchild, last_pack, sizeof(last_pack),0, client, *length);
	
				break;
			}

			else
			{


				char packet[516];
				char* data_packet = packet;
				uint16_t opcode = DATA;
				uint16_t bl=htons(block);
				opcode=htons(opcode);
				memcpy(data_packet, &opcode, sizeof(uint16_t));
				data_packet+=sizeof(uint16_t);
				memcpy(data_packet, &bl, sizeof(uint16_t));
				data_packet+=(sizeof(uint16_t));
				memcpy(data_packet, &read_buf, sizeof(read_buf));
	

				sendto(sdchild, packet, sizeof(packet),0, client, *length);
	

				char ack_packet[4];

				alarm(1);
				recvfrom(sdchild, ack_packet, 4, 0, client, length );
				alarm(0); 
				timeouts = 0;

				char ack_code[2];
				char blockrecv[2];
				ack_code[0] = ack_packet[0];
				ack_code[1] = ack_packet[1];
				blockrecv[0] = ack_packet[2];
				blockrecv[1] = ack_packet[3];
				uint16_t * ackcode_ptr = (uint16_t *) ack_code;
				uint16_t ack = ntohs(*ackcode_ptr); 
				uint16_t * blockrecvptr = (uint16_t *) blockrecv;
				uint16_t b = ntohs(*blockrecvptr);
				printf("%d %d \n", ack, b); 
				
				
				block++;
			}

		}

	}	

}



void handle_write_request(char* filename, struct sockaddr * client, socklen_t* length)
{	


	int sdchild = socket( AF_INET, SOCK_DGRAM, 0 ) ; 
	struct sockaddr_in serverchild;
	int length_child = sizeof(serverchild);

	if ( sdchild < 0 )  
	{
		perror( "socket() failed" );
    		
	}

	bzero(&serverchild, length_child);
	serverchild.sin_family = AF_INET;
	serverchild.sin_addr.s_addr = htonl(INADDR_ANY);
	serverchild.sin_port = htons(0); 

	if ( bind( sdchild, (struct sockaddr *) &serverchild, length_child ) < 0 )
 	{
   		perror( "bind() failed" );
   		
  	}

	FILE * file;
	file = fopen(filename, "wb");

	uint16_t block=0;

	char ack_packet[4];
	char* ack_ptr = ack_packet;
	bzero(ack_packet,4);
	uint16_t opcode = ACK;
	uint16_t bl=htons(block);
	opcode=htons(opcode);
	memcpy(ack_packet, &opcode, sizeof(uint16_t));
	ack_ptr += sizeof(uint16_t);
	memcpy(ack_packet, &bl, sizeof(uint16_t));
	sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length);	
	block++;


	while (true)
	{

		char packet[516];

		alarm(1);
		int bytes_recieved = recvfrom(sdchild, packet, 516, 0, client, length);	
		alarm(0);
		timeouts = 0;

		printf("%d bytes written to file\n", bytes_recieved);	
			
		if (bytes_recieved < 516)
		{

			char write_buf[bytes_recieved - 4];
			bzero(write_buf,bytes_recieved - 4);
			for(int i = 4; i < bytes_recieved; i++) {
				write_buf[i - 4] = packet[i];
			}

			cout << write_buf;

			printf("%d bytes left. This is last packet\n", bytes_recieved);
			fwrite(write_buf,sizeof(char),bytes_recieved - 4,file);

			bzero(ack_packet,4);
			ack_ptr = ack_packet;
			opcode = ACK;
			bl=htons(block);
			opcode=htons(opcode);
			memcpy(ack_packet, &opcode, sizeof(uint16_t));
			ack_ptr += sizeof(uint16_t);
			memcpy(ack_packet, &bl, sizeof(uint16_t));
			sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length);
	
			break;
		}

		else
		{
			
			char write_buf[512];
			bzero(write_buf,512);
			for(int i = 4; i < 516; i++) {
				write_buf[i - 4] = packet[i];
			}
			fwrite(write_buf,sizeof(char),512,file);


			bzero(ack_packet,4);
			ack_ptr = ack_packet;
			opcode = ACK;
			bl=htons(block);
			opcode=htons(opcode);
			memcpy(ack_packet, &opcode, sizeof(uint16_t));
			ack_ptr += sizeof(uint16_t);
			memcpy(ack_packet, &bl, sizeof(uint16_t));
			sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length);

			block++;
		}

	}
	

}


 

 
 
 
int main (int argc, char* argv[])
{

	signal(SIGALRM, handle_alarm);

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

	char buffer[1024];
	bzero(buffer,1024);
	char codebuff[2];
	uint16_t opcode;		
	while (1)
	{
	
		int bytes_read = recvfrom( sd, buffer, 1024, 0, (struct sockaddr *) &client,
                  (socklen_t *) &leng );

	
		if (bytes_read > 0)
		{
			
			codebuff[0] = buffer[0];
			codebuff[1] = buffer[1];
			uint16_t* opcode_ptr =  (uint16_t*) codebuff;
			opcode = ntohs(*opcode_ptr);
			if(opcode != RRQ && opcode != WRQ) 
			{
				string message = "Illegal TFTP Operation";
				uint16_t errcode= 4;
				handle_error(sd,(struct sockaddr *) &client,(socklen_t *) &length,errcode,message);
			}

			
			else 
			{
            	if(fork() == 0) 
				{
                	/* Child - handle the request */
                	close(sd);
                	break;
            	}

			
			    else 
				{
				/* Parent - continue to wait */
			    }
			}
         
		}
	}

	if (opcode == RRQ)
	{ 
			
		char readfile[1024];
		for (int i=2; i < 1024; i++)
		{
			if (buffer[i] == '\0') 
			{

				readfile[i-2] = '\0';
				handle_read_request(readfile, (struct sockaddr *) &client, (socklen_t *) &leng);
				break;

			}	

			else
			{
				readfile[i-2] = buffer[i];
			}

		}
	}

	if (opcode == WRQ) 
	{

		char writefile[1024];
		for(int i=2; i < 1024; i++) 
		{
			if(buffer[i] == '\0')
			{
				writefile[i-2] = '\0';
				handle_write_request(writefile, (struct sockaddr *) &client, (socklen_t *) &leng);
				break;
			}

			else
			{
				writefile[i-2] = buffer[i];
			}


		}

	}



	return 0;

}
