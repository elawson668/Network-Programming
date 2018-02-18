#include <sys/types.h>
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
#include <sys/wait.h>

#define RRQ 1
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5


int count = 0;

void sigchild(int signo)
{
	//standard code from Lecture 3 slides to guarantee child termination successfully
	pid_t pid;
	int stat;
	while ((pid=waitpid(-1, &stat, WNOHANG)) >0)
	{printf("%d child terminated\n", pid);}
	return;
}

// Increment count for timeout
void handle_alarm(int signum) 
{
	printf("%d\n", count); count+=1;
}

/*
// Create error packet and send to client
void handle_error(int sd, struct sockaddr * client, socklen_t* length, uint16_t errcode, std::string message) {

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
*/

//Function that is used to handle an incoming packet with a RRQ opcode
//return value: void
//Parameters:
//filename: name of the file we are attemtping to read
//client: pointer to the incomign client connection
//length: size of the client
//port: incoming TID for the incoming client
void handle_read_request(char* filename, struct sockaddr * client, socklen_t* length, int port)
{	
	
	// Open new socket for child process
	
	int sdchild = socket( AF_INET, SOCK_DGRAM, 0 ) ; 
	struct sockaddr_in serverchild;
	int length_child = sizeof(serverchild);

	if ( sdchild < 0 )  
	{
		perror( "socket() failed" );
    		
	}

	//assign it to a random OS bound port number for binding, accept any IP address
	bzero(&serverchild, length_child);
	serverchild.sin_family = AF_INET;
	serverchild.sin_addr.s_addr = htonl(INADDR_ANY);
	serverchild.sin_port = htons(0); 


	//bind child to new port for connection
	if ( bind( sdchild, (struct sockaddr *) &serverchild, length_child ) < 0 )
 	{
   		perror( "bind() failed" );
   		
  	}

  	// Open file requested by client
	FILE* file_to_read = fopen(filename, "rb");

	// Check for errors
	if (file_to_read == NULL) 
	{	
		//file does not exist, send file not found error ot client
		if(errno == ENOENT)
		{

			char no_file_err[4 + sizeof("FILE NOT FOUND")]; //error message packet
			char* no_f_ptr = no_file_err; //pointer to the packet
			uint16_t opcode = ERROR;
			uint16_t err_code= 1;
			opcode = htons(opcode); //set opcode to be equal to ERROR
			err_code = htons(err_code); //set appropriate error code for this error
			memcpy(no_f_ptr, &opcode, sizeof(uint16_t)); //sift pointer to the left two bytes twice to add opcode and error code to packet
			no_f_ptr += sizeof(uint16_t);
			memcpy(no_f_ptr, &err_code, sizeof(uint16_t));
			no_f_ptr+=sizeof(uint16_t); //shift point again to copy eror message into remianign space of packet, send packet to client
			memcpy(no_f_ptr, "FILE NOT FOUND", sizeof("FILE NOT FOUND"));
			sendto(sdchild, no_file_err, sizeof(no_file_err),0, client, *length);

			/*
			uint16_t err_code= 1;
			std::string message = "FILE NOT FOUND";
			handle_error(sdchild,client,length,err_code,message);
			*/
		}

		//file cannot be accessed due to permissions issues, send access violation error packet to client
		else if(access(filename, R_OK) != 0){

			char err[4+sizeof( "ACCESS VIOLATION")]; //error packet with space for message
			char * errptr = err;
			uint16_t opcode = ERROR; //error opcode
			uint16_t err_code= 2; //error code for access violation
			opcode = htons(opcode);
			err_code = htons(err_code);
			memcpy(errptr, &opcode, sizeof(uint16_t)); //copy opcode to packet, then shift 2 bytes
			errptr += sizeof(uint16_t);
			memcpy(errptr, &err_code, sizeof(uint16_t)); //copy error code to packet, shift 2 bytes
			errptr+=sizeof(uint16_t);
			memcpy(errptr, "ACCESS VIOLATION", sizeof("ACCESS VIOLATION")); //copy error message to remaining space and send to client
			sendto(sdchild, err, sizeof(err),0, client, *length);

			/*
			uint16_t err_code= 2;
			std::string message = "ACCESS VIOLATION";
			handle_error(sdchild,client,length,err_code,message);
			*/
		}
			
	}

	//the file can be read in, so we read it
	else
	{

		char read_buf[512]; //space to store waht we read in
		bzero(read_buf, 512);
		uint16_t block=1; //start off with block number 1

		//keep going while we have data
		while (true)
		{
			// Read bytes from file 512 bytes at a time
			int bytes_read = fread(read_buf, 1, 512, file_to_read);
			printf("%d bytes read from file\n", bytes_read);



			//come back here to resend the data packet to the client if we hit a 1 secodn timeout
			resend:

			// Send last DATA packet of file to client, we read less than 512 bytes
			if (bytes_read < 512)
			{
				printf("%d bytes left. This is last packet\n", bytes_read); 
				char last_pack[bytes_read+4]; //4 bytes plus unknown size of bytes read in from fiel less than 512
				bzero(last_pack, bytes_read);
				char* last_ptr = last_pack;
				uint16_t opcode = DATA;
				opcode=htons(opcode);
				block=htons(block);
				memcpy(last_ptr, &opcode, sizeof(uint16_t)); //copy data opcode, sift 2 bytes in packet
				last_ptr+=sizeof(uint16_t);
				memcpy(last_ptr, &block, sizeof(uint16_t)); //copy block number, shift 2 bytes
				last_ptr+=sizeof(uint16_t);
				memcpy(last_ptr, &read_buf, bytes_read); //copy reamining data into packet, send to client, break loop because we are done
				sendto(sdchild, last_pack, sizeof(last_pack),0, client, *length);
	
				break;
			}

			else
			{
				// Send DATA packet of 512 bytes to client, not last pakcet
				char packet[516]; //516 total bytes in a packet
				char* data_packet = packet;
				uint16_t opcode = DATA;
				uint16_t bl=htons(block);
				opcode=htons(opcode);
				memcpy(data_packet, &opcode, sizeof(uint16_t)); //copy in DATA opcode, shift 2 bytes
				data_packet+=sizeof(uint16_t);
				memcpy(data_packet, &bl, sizeof(uint16_t)); //copy block num into next two bytes
				data_packet+=(sizeof(uint16_t));
				memcpy(data_packet, &read_buf, sizeof(read_buf)); //copy 5212 bytes and send complete packet
				
				sendto(sdchild, packet, sizeof(packet),0, client, *length);
				

				//come back here with a goto if the ACK isn't the correct packet we are expecting
				get_right_ack:

				// Check for timeout
				char ack_packet[4];
				alarm(1);
				if (count ==10) {printf("Timeout of 10 seconds!\n"); break;}

				// Recieve ACK packet from client, if SIGALRM is thrown, this will return EINTR
				int bytes_rec= recvfrom(sdchild, ack_packet, 4, 0, client, length );
				
				if (bytes_rec < 0)
				{
					if(errno == EINTR) goto resend; //SIGALRM or system interrupt means we try to resend data
            			perror("recvfrom");
            			exit(-1);
            			}

				alarm(0); //recvfrom worked, cancel the alarm
				count=0; //resent timeout count to 0

				
				struct sockaddr_in* s = (struct sockaddr_in*) client;
				
				if( port != ntohs(s->sin_port)) //check port of incoming client, if not what we agreed onin step 1, throw an error packet but keep going

				{

					char err[4+sizeof( "UNKNOWN TRANSFER ID")]; //unkonw transfer ID error packet
					char * errptr = err;
					uint16_t opcode = ERROR;
					uint16_t err_code= 5;
					opcode = htons(opcode);
					err_code = htons(err_code); 
					memcpy(errptr, &opcode, sizeof(uint16_t)); //copy in ocode, shift 2 bytes
					errptr += sizeof(uint16_t);
					memcpy(errptr, &err_code, sizeof(uint16_t)); //copy in erro code, shift 2 bytes
					errptr+=sizeof(uint16_t);
					memcpy(errptr, "UNKNOWN TRANSFER ID", sizeof("UNKNOWN TRANSFER ID")); //copy in error message, send to bad client
					sendto(sdchild, err, sizeof(err),0, client, *length);
					goto get_right_ack;	//go to block to wait for the correct ack packet from the right client
					

				}
				

				//if we get this far, the data was good enough to be read in

				char ack_code[2]; //buffer to chekc opcode
				char blockrecv[2]; //buffer to check block number
				ack_code[0] = ack_packet[0];
				ack_code[1] = ack_packet[1];
				blockrecv[0] = ack_packet[2];
				blockrecv[1] = ack_packet[3];
				uint16_t * ackcode_ptr = (uint16_t *) ack_code; 
				uint16_t ack = ntohs(*ackcode_ptr); 
				uint16_t * blockrecvptr = (uint16_t *) blockrecv;
				uint16_t b = ntohs(*blockrecvptr);
				
				// Check for Sorcerer's Apprentice bug, make sure we are only sending an ACK if it is for the next data block, not previous ones
				//also make sure we are receiving an ACK packet
				if (ack==ACK && b==block){}
				else{ printf("DUPLICATE PACKET RECEIVED\n"); goto get_right_ack;}
				
				
				block++; //increment block number if the receive was successful
			}

		}

	}	

	fclose(file_to_read); //we are out of loop, file can be close now

}



//Function that is used to handle an incoming packet with a WRQ opcode
//return value: void
//Parameters:
//filename: name of the file we are attemtping to read
//client: pointer to the incomign client connection
//length: size of the client
//port: incoming TID for the incoming client
void handle_write_request(char* filename, struct sockaddr * client, socklen_t* length, int port)
{	

	// Open new socket for child process
	int sdchild = socket( AF_INET, SOCK_DGRAM, 0 ) ; 
	struct sockaddr_in serverchild;
	int length_child = sizeof(serverchild);

	if ( sdchild < 0 )  
	{
		perror( "socket() failed" );
    		
	}

	//setup newchild info
	bzero(&serverchild, length_child);
	serverchild.sin_family = AF_INET;
	serverchild.sin_addr.s_addr = htonl(INADDR_ANY);
	serverchild.sin_port = htons(0); 


	//bind new child to socket
	if ( bind( sdchild, (struct sockaddr *) &serverchild, length_child ) < 0 )
 	{
   		perror( "bind() failed" );
   		
  	}



	FILE * file;

	// Check if file already exists, in currecnt directory
	if(access(filename, F_OK) == 0)
	{

		char already[4+sizeof("FILE ALREADY EXISTS")]; //error packet
		char* aptr = already;
		uint16_t opcode = ERROR;
		uint16_t err_code= 6;
		opcode = htons(opcode);
		err_code = htons(err_code);
		memcpy(aptr, &opcode, sizeof(uint16_t)); //copy opcode to error packet
		aptr += sizeof(uint16_t);
		memcpy(aptr, &err_code, sizeof(uint16_t)); //copy error code to erorr packet
		aptr+=sizeof(uint16_t);
		memcpy(aptr, "FILE ALREADY EXISTS", sizeof("FILE ALREADY EXISTS")); //copy erorr message, send to client then end the request
		sendto(sdchild, already, sizeof(already),0, client, *length);
		return;


		/*
		uint16_t err_code= 6;
		std::string message = "FILE ALREADY EXISTS";
		handle_error(sdchild,client,length,err_code,message);
		return;
		*/

	}



	file = fopen(filename, "wb");

	

	// Send inital ACK packet to client
	uint16_t block=0;
	char ack_packet[4]; //packet for ACK that we willl send, only contains opcode and block number
	char* ack_ptr = ack_packet;
	bzero(ack_packet,4);
	uint16_t opcode = ACK; 
	uint16_t bl=htons(block); //network support for block number
	opcode=htons(opcode);
	memcpy(ack_ptr, &opcode, sizeof(uint16_t)); //copy in opcode, shift 2 bytes
	ack_ptr+= sizeof(uint16_t);
	memcpy(ack_ptr, &bl, sizeof(uint16_t)); //copy in block number, shift 2 bytes
	block++; //increment block number from 0 so all subsequent blocks will start at 1

	//resend ack packets in case of a timeout
	//if ACK 0 did not go through, ack_packet will be resent  as it was constructed here with block number of 0
	//for all subsequent cases, ack_packet is modified depending on what block number we are on, it will be more than 0 if we get at least one data pakcet
	resend_data:
	sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length);	
	

	//while we still have data to receive
	while (true)
	{
		//goto if block numberis wrong or we hit a timeout of 1 second
		get_right_data:
		char packet[516]; //storage for data coming in
		alarm(1);
		// Check for timeout, if we hit 10 we are done
		if (count ==10) {printf("Timeout of 10 seconds!\n"); break;}

		// Recieve DATA packet of 512 bytes from client
		int bytes_recieved = recvfrom(sdchild, packet, 516, 0, client, length);	

		if (bytes_recieved < 0){
			if(errno == EINTR) goto resend_data; //either SIGALRM got thrown or system was interrupted, either way, wait for the packet again
           		 perror("recvfrom");
           		 exit(-1);
        	}

		alarm(0); //we successfully got data, reset counter, cancel alarm
		count=0;


		struct sockaddr_in* s = (struct sockaddr_in*) client;
				
				if( port != ntohs(s->sin_port)) //check to see if client port number matches established TID

				{

					char err[4+sizeof( "UNKNOWN TRANSFER ID")];
					char * errptr = err;
					uint16_t opcode = ERROR;
					uint16_t err_code= 5;
					opcode = htons(opcode);
					err_code = htons(err_code);
					memcpy(errptr, &opcode, sizeof(uint16_t)); //error opcode copied, shft 2 bytes
					errptr += sizeof(uint16_t);
					memcpy(errptr, &err_code, sizeof(uint16_t)); //error code copied, shift 2 bytes
					errptr+=sizeof(uint16_t);
					memcpy(errptr, "UNKNOWN TRANSFER ID", sizeof("UNKNOWN TRANSFER ID")); //copy message, send error to client and go back to waiting for good data
					sendto(sdchild, err, sizeof(err),0, client, *length);
					goto get_right_data;	
					

				}	

	
		
		
	
		// If last packet
		if (bytes_recieved < 516)
		{

			// Write final bytes to file
			char write_buf[bytes_recieved - 4];
			bzero(write_buf,bytes_recieved - 4);
			for(int i = 4; i < bytes_recieved; i++) {
				write_buf[i - 4] = packet[i];
			}

			printf("%d bytes left. This is last packet\n", bytes_recieved);
			fwrite(write_buf,sizeof(char),bytes_recieved - 4,file);

			// Send final ACK packet
			bzero(ack_packet,4);
			ack_ptr=ack_packet;
			opcode = ACK;
			bl=htons(block);
			opcode=htons(opcode);
			memcpy(ack_ptr, &opcode, sizeof(uint16_t)); //copy ACK opcode
			ack_ptr+= sizeof(uint16_t);
			memcpy(ack_ptr, &bl, sizeof(uint16_t)); //copy block number of current block and sned it
			sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length); 

	
			break; //break loop, nno more data
		}

		else
		{
			
			// Write 512 bytes to file
			char write_buf[512];
			bzero(write_buf,512);
			for(int i = 4; i < 516; i++) {
				write_buf[i - 4] = packet[i];
			}
			fwrite(write_buf,sizeof(char),512,file);
			printf("%d bytes written to file\n", bytes_recieved-4);	
			// Send ACK packet to client
			bzero(ack_packet,4);
			ack_ptr=ack_packet;
			opcode = ACK;
			bl=htons(block);
			opcode=htons(opcode);
			memcpy(ack_ptr, &opcode, sizeof(uint16_t)); //copy ACK opcode, shift 2 bytes
			ack_ptr+= sizeof(uint16_t);
			memcpy(ack_ptr, &bl, sizeof(uint16_t)); //copy block number, send ack to client
			sendto(sdchild, ack_packet, sizeof(ack_packet),0, client, *length);

			block++; //increment block number for the next iteration
		}

	}

	fclose(file); //close the file, we are done
	
}

//Main function
int main (int argc, char* argv[])
{
	
	signal(SIGALRM, handle_alarm); //set deafult behavior of SIGALRM to increment thetimeout counter
	signal(SIGCHLD, sigchild); //handle behavior for termination of child process
	siginterrupt(SIGALRM, 1); //ensure that any SIGALRMSthat get thrown prevent recvfrom and sendto from restarting, guarantees EINTR will be thrown to increment counter
	int sd; 
	struct sockaddr_in udpserver; //server socker
	int length = sizeof(udpserver);

	sd = socket( AF_INET, SOCK_DGRAM, 0 );  //set up server socket

	if ( sd < 0 )  
	{
		perror( "socket() failed" );
    	return EXIT_FAILURE;
	}

	bzero(&udpserver, length);
	udpserver.sin_family = AF_INET;
	udpserver.sin_addr.s_addr = htonl(INADDR_ANY); //any IP address
	udpserver.sin_port = htons(0);  //random OS assigned port number to start out with

	if ( bind( sd, (struct sockaddr *) &udpserver, length ) < 0 ) //bind socket
  	{
   		perror( "bind() failed" );
   		return EXIT_FAILURE;
  	}


  	if ( getsockname( sd, (struct sockaddr *) &udpserver, (socklen_t *) &length ) < 0 ) 
 	{
   		perror( "getsockname() failed" );
   		return EXIT_FAILURE;
 	}

 	printf( "TFTP server assigned to port number %d\n", ntohs( udpserver.sin_port ) ); //print out port number to stdout


	struct sockaddr_in client; //struct for incoming client
	int leng = sizeof( client );

	char buffer[1024]; //buffer, we can read in a maximum initial operation of 1024 bytes for RRQ and WRQ
	bzero(buffer,1024); //zero out the buffer
	char codebuff[2]; //buffer to check the opcodeof what we got
	uint16_t opcode;		


	while (1) //server loops forever
	{
	
	//block if intial recvfrom gets interrupted
	intr_send:
		
		// Recieve request packet from client 
		int bytes_read = recvfrom( sd, buffer, 1024, 0, (struct sockaddr *) &client,
                  (socklen_t *) &leng );

		if(bytes_read < 0) //error handling if we got no data
		{
            if(errno == EINTR) goto intr_send;
            perror("recvfrom");
            exit(-1);
        }
		
		if (bytes_read > 0)  //we got data, check the opcode and spawn a child
		{
			
			codebuff[0] = buffer[0]; //ceck first two bytes, convert it to valid opcode for checking
			codebuff[1] = buffer[1];
			uint16_t* opcode_ptr =  (uint16_t*) codebuff;
			opcode = ntohs(*opcode_ptr);

			// Check for illegal TFTP operation
			if(opcode != RRQ && opcode != WRQ) 
			{

				char illegal[4+sizeof("ILLEGAL OPERATION")]; //illegal operation packet
				char* ill_ptr = illegal;
            	uint16_t eopcode = ERROR;
				uint16_t err_code= 4;
				eopcode = htons(eopcode);
				err_code = htons(err_code);
				memcpy(ill_ptr, &eopcode, sizeof(uint16_t)); //copy in error opcode
				ill_ptr += sizeof(uint16_t);
				memcpy(ill_ptr, &err_code, sizeof(uint16_t)); //copy in eror code for illegal operation
				ill_ptr+=sizeof(uint16_t);
				memcpy(ill_ptr, "ILLEGAL OPERATION", sizeof("ILLEGAL OPERATION")); //copy error message, send to client, server keep running
				sendto(sd, illegal, sizeof(illegal),0, (struct sockaddr *) &client, leng);				

				/*
				uint16_t err_code= 4;
				std::string message = "ILLEGAL TFTP OPERATION";
				handle_error(sd,(struct sockaddr *) &client,(socklen_t *) &length,err_code,message);
				*/

			}

			else //we got an opcode ofRRQ or WRQ
			{
				// Create new process for each connection
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

	// Read request handler outsid eof server loop
	if (opcode == RRQ)
	{ 
			
		char readfile[1024];
		for (int i=2; i < 1024; i++) //check bytes after opcode up to the null terminator for the filename, deafault mode is octet so that is never checked
		{
			if (buffer[i] == '\0') //we have the file name, send it to readrequest function
			{

				readfile[i-2] = '\0';
				handle_read_request(readfile, (struct sockaddr *) &client, (socklen_t *) &leng, ntohs(client.sin_port));
				break;

			}	

			else
			{
				readfile[i-2] = buffer[i]; //kepp copying file name one byte at a time
			}

		}
	}

	// Write request
	if (opcode == WRQ) 
	{

		char writefile[1024];
		for(int i=2; i < 1024; i++) 
		{
			if(buffer[i] == '\0') //read bytes after opcode up to null erminator for the filename, deafault mode is octet so that is never checked
			{
				writefile[i-2] = '\0';
				handle_write_request(writefile, (struct sockaddr *) &client, (socklen_t *) &leng, ntohs(client.sin_port));
				break;
			}

			else
			{
				writefile[i-2] = buffer[i];
			}


		}

	}

	close(sd);
	return 0;

}
