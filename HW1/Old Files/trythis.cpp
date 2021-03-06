#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
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


int count=0;

void handle_alarm(int signum) {printf("%d\n", count); count+=1;}


void sigchild(int signo)
{

	pid_t pid;
	int stat;
	while ((pid=waitpid(-1, &stat, WNOHANG)) >0)
	{printf("%d child terminated\n", pid);}

	return;
	




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






	
	FILE* file_to_read = fopen(filename, "rb");



	
	if (file_to_read == NULL) 
	{	
		if(errno == ENOENT)
		{
		char no_file_err[4 + sizeof("FILE NOT FOUND")];
	char* no_f_ptr = no_file_err;
		uint16_t opcode = ERROR;
		uint16_t err_code= 1;
		opcode = htons(opcode);
		err_code = htons(err_code);
		memcpy(no_f_ptr, &opcode, sizeof(uint16_t));
		no_f_ptr += sizeof(uint16_t);
		memcpy(no_f_ptr, &err_code, sizeof(uint16_t));
		no_f_ptr+=sizeof(uint16_t);
		memcpy(no_f_ptr, "FILE NOT FOUND", sizeof("FILE NOT FOUND"));
		sendto(sdchild, no_file_err, sizeof(no_file_err),0, client, *length);}


		else if(access(filename, R_OK) != 0){

		char err[4+sizeof( "ACCESS VIOLATION")];
		char * errptr = err;
		uint16_t opcode = ERROR;
		uint16_t err_code= 2;
		opcode = htons(opcode);
		err_code = htons(err_code);
		memcpy(errptr, &opcode, sizeof(uint16_t));
		errptr += sizeof(uint16_t);
		memcpy(errptr, &err_code, sizeof(uint16_t));
		errptr+=sizeof(uint16_t);
		memcpy(errptr, "ACCESS VIOLATION", sizeof("ACCESS VIOLATION"));
		sendto(sdchild, err, sizeof(err),0, client, *length);




		}

					
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
			resend:
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
				

				get_right_ack:
				char ack_packet[4];
				alarm(1);
				if (count ==10) {printf("Timeout of 10 seconds, aborting...\n"); break;}
				int bytes_rec= recvfrom(sdchild, ack_packet, 4, 0, client, length );
				
				if (bytes_rec < 0){
				if(errno == EINTR) goto resend;
            			perror("recvfrom");
            			exit(-1);}

				alarm(0);
				count=0;
				
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
				
				if (ack==ACK && b==block){}
				else{ printf("%d %d\n", ack, b); goto get_right_ack;}
				
				
		
				
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

	if(access(filename, F_OK) == 0)
	{

		char already[4+sizeof("FILE ALREADY EXISTS")];
		char* aptr = already;
		uint16_t opcode = ERROR;
		uint16_t err_code= 6;
		opcode = htons(opcode);
		err_code = htons(err_code);
		memcpy(aptr, &opcode, sizeof(uint16_t));
		aptr += sizeof(uint16_t);
		memcpy(aptr, &err_code, sizeof(uint16_t));
		aptr+=sizeof(uint16_t);
		memcpy(aptr, "FILE ALREADY EXISTS", sizeof("FILE ALREADY EXISTS"));
		sendto(sdchild, already, sizeof(already),0, client, *length);
		return;


	}





	file = fopen(filename, "wb");
	resend_data:
	uint16_t block=0;
	char ack_packet[4];
	char* ack_ptr = ack_packet;
	bzero(ack_packet,4);
	uint16_t opcode = ACK;
	uint16_t bl=htons(block);
	opcode=htons(opcode);
	memcpy(ack_ptr, &opcode, sizeof(uint16_t));
	ack_ptr+= sizeof(uint16_t);
	memcpy(ack_ptr, &bl, sizeof(uint16_t));
	sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length);	
	block++;


	while (true)
	{

		char packet[516];
		alarm(1);
		if (count ==10) {printf("Timeout of 10 seconds, aborting...\n"); break;}
		int bytes_recieved = recvfrom(sdchild, packet, 516, 0, client, length);	
		if (bytes_recieved < 0){
		if(errno == EINTR) goto resend_data;
            	perror("recvfrom");
            	exit(-1);}
		alarm(0);
		count=0;	
		printf("%d bytes written to file\n", bytes_recieved);	
			
		if (bytes_recieved < 516)
		{

			char write_buf[bytes_recieved - 4];
			bzero(write_buf,bytes_recieved - 4);
			for(int i = 4; i < bytes_recieved; i++) {
				write_buf[i - 4] = packet[i];
			}

			

			printf("%d bytes left. This is last packet\n", bytes_recieved);
			fwrite(write_buf,sizeof(char),bytes_recieved - 4,file);



			bzero(ack_packet,4);
			ack_ptr=ack_packet;
			opcode = ACK;
			bl=htons(block);
			opcode=htons(opcode);
			memcpy(ack_ptr, &opcode, sizeof(uint16_t));
			ack_ptr+= sizeof(uint16_t);
			memcpy(ack_ptr, &bl, sizeof(uint16_t));
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
			ack_ptr=ack_packet;
			opcode = ACK;
			bl=htons(block);
			opcode=htons(opcode);
			memcpy(ack_ptr, &opcode, sizeof(uint16_t));
			ack_ptr+= sizeof(uint16_t);
			memcpy(ack_ptr, &bl, sizeof(uint16_t));
			sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length);

			block++;
		}

	}
	

}


 

 
 
 
int main (int argc, char* argv[])
{
	
	signal(SIGALRM, handle_alarm);
	signal(SIGCHLD, sigchild);
	siginterrupt(SIGALRM, 1);
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
	
	intr_send:
		
		int bytes_read = recvfrom( sd, buffer, 1024, 0, (struct sockaddr *) &client,
                  (socklen_t *) &leng );
		if(bytes_read < 0) 
		{
            		if(errno == EINTR) goto intr_send;
            		perror("recvfrom");
            		exit(-1);
        	}
		
		if (bytes_read > 0)
		{
			
			codebuff[0] = buffer[0];
			codebuff[1] = buffer[1];
			uint16_t* opcode_ptr =  (uint16_t*) codebuff;
			opcode = ntohs(*opcode_ptr);
			if(opcode != RRQ && opcode != WRQ) 
			{
				char illegal[4+sizeof("ILLEGAL OPERATION")];
				char* ill_ptr = illegal;
            			uint16_t eopcode = ERROR;
				uint16_t err_code= 4;
				eopcode = htons(eopcode);
				err_code = htons(err_code);
				memcpy(ill_ptr, &eopcode, sizeof(uint16_t));
				ill_ptr += sizeof(uint16_t);
				memcpy(ill_ptr, &err_code, sizeof(uint16_t));
				ill_ptr+=sizeof(uint16_t);
				memcpy(ill_ptr, "ILLEGAL OPERATION", sizeof("ILLEGAL OPERATION"));
				sendto(sd, illegal, sizeof(illegal),0, (struct sockaddr *) &client, leng);

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
