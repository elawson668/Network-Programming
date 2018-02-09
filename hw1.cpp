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

using namespace std;

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5

void handle_read_request(char* filename, int sd, struct sockaddr * client, socklen_t* length)
{	
	char no_file_err[4 + sizeof("FILE NOT FOUND")];
	char* no_f_ptr = no_file_err;
	FILE* file_to_read = fopen(filename, "r");
	if (file_to_read == NULL) 
	{	
		uint16_t opcode = ERROR;
		uint16_t err_code= 1;
		opcode = htons(opcode);
		err_code = htons(err_code);
		memcpy(no_f_ptr, &opcode, sizeof(uint16_t));
		no_f_ptr += sizeof(uint16_t);
		memcpy(no_f_ptr, &err_code, sizeof(uint16_t));
		no_f_ptr+=sizeof(uint16_t);
		memcpy(no_f_ptr, "FILE NOT FOUND", sizeof("FILE NOT FOUND"));
		sendto(sd, no_file_err, sizeof(no_file_err),0, client, *length);

					
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
				sendto(sd, last_pack, sizeof(last_pack),0, client, *length);
	
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
	
				sendto(sd, packet, sizeof(packet),0, client, *length);
	


				char ack_packet[4];
				
				recvfrom( sd, ack_packet, 4, 0, client, length );
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


void handle_write_request(char* filename, int sd, struct sockaddr * client, socklen_t* length)
{	

	FILE * file;
	file = fopen(filename, "wb");

	uint16_t block=1;
	while (true)
	{

		char packet[516];
		int bytes_recieved = recvfrom(sd, packet, 516, 0, client, length);		
		
		if (bytes_recieved < 516)
		{

			char write_buf[bytes_recieved - 4];
			bzero(write_buf,bytes_recieved - 4);
			for(int i = 4; i < bytes_recieved; i++) {
				write_buf[i - 4] = packet[i];
			}

			printf("%d bytes left. This is last packet\n", bytes_recieved);
			fwrite(write_buf,sizeof(char),bytes_recieved - 4,file);
	
			break;
		}

		else
		{
			
			char write_buf[512];
			bzero(write_buf,512);
			for(int i = 4; i < 516; i++) {
				write_buf[i - 4] = packet[i];
			}
			printf("%d bytes written to file\n", bytes_recieved);
			fwrite(write_buf,sizeof(char),512,file);


			char ack_packet[4];
			uint16_t opcode = ACK;
			uint16_t bl=htons(block);
			opcode=htons(opcode);
			memcpy(ack_packet, &opcode, sizeof(uint16_t));
			memcpy(ack_packet, &bl, sizeof(uint16_t));
			sendto(sd, ack_packet, sizeof(ack_packet),0, client, *length);

			block++;
		}

	}
	

}


 

 
 
 
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
		bzero(buffer,1024);
		int bytes_read = recvfrom( sd, buffer, 1024, 0, (struct sockaddr *) &client,
                  (socklen_t *) &leng );

	
		if (bytes_read > 0)
		{

			uint16_t opcode = buffer[0] + buffer[1];
			if (opcode == 1)
			{ 
			
				char readfile[1024];
				for (int i=2; i < 1024; i++)
				{
					if (buffer[i] == '\0') 
					{

						readfile[i-2] = '\0';
						handle_read_request(readfile, sd, (struct sockaddr *) &client, (socklen_t *) &leng);
						break;

					}	

					else
					{
						readfile[i-2] = buffer[i];
					}

				}
			}

			if (opcode == 2) 
			{

				char writefile[1024];
				for(int i=2; i < 1024; i++) 
				{
					if(buffer[i] == '\0')
					{
						writefile[i-2] = '\0';
						handle_write_request(writefile,sd,(struct sockaddr *) &client, (socklen_t *) &leng);
						break;
					}

					else
					{
						writefile[i-2] = buffer[i];
					}


				}

			}
		}
	}

	return 0;

}
