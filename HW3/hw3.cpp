
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
#include <algorithm>



using namespace std;


#define LONG_TIME 100000000

int client_fds[100];
char buffer[1024];    

map<string,vector<int> > user_channels;
map<int,string> users;
map<string, int> nametofd;
map<int,bool> operators;
vector<string> server_channels;
string password;
bool ispassword = false;


static volatile int timeOut = LONG_TIME;

bool regex(string word) {

	int n = word.length();
	char w[n];
	strcpy(w,word.c_str());

	if(!isalpha(w[0])) return false;

	for(int i = 1; i < n; i++) {
		if(!isalpha(w[i]) && !isdigit(w[i]) && w[i] != '_') return false;
	}

	return true;
}



int main(int argc, char* argv[])
{

    if(argc == 2)
    {
	string flag = argv[1];
	size_t equals = flag.find_first_of('=');

	if (flag.substr(0,equals+1) == "--opt-pass=") 
	{
		password = flag.substr(equals+1);

		if(!regex(password)) {
			cout << "Startup failed - invalid password.\n";
			return 0;
		}

		ispassword=true;
	}



    }
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
					
					nametofd.erase(users[fd]);
					users.erase(fd);
					operators.erase(fd);
						

				}

				else
				{

					
					buffer[incoming_bytes]='\0';

					string buf = buffer;

					if (users.find(fd) == users.end()) {


						if(buf.substr(0,5).compare("USER ") == 0) {

							string name = buf.substr(5);
							name.erase(remove(name.begin(), name.end(), '\n'), name.end());
							if(name.length() > 20) {
								send(fd, "Invalid username\n", sizeof("Invalid username\n"),0);
								bzero(buffer,1024);
							
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

								nametofd.erase(users[fd]);
								users.erase(fd);
								operators.erase(fd);

								continue;
							}

							if(!regex(name)) {
								send(fd, "Invalid username\n", sizeof("Invalid username\n"),0);
								bzero(buffer,1024);

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

								nametofd.erase(users[fd]);
								users.erase(fd);
								operators.erase(fd);

								continue;

							}


							users.insert(pair<int,string>(fd,name));
							operators.insert(pair<int,bool>(fd,false));
							nametofd.insert(pair<string,int>(name,fd));
						
						}

						else {
							send(fd, "Invalid command, please identify yourself with USER.\n", 
							sizeof("Invalid command, please identify yourself with USER.\n"),0);
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
							nametofd.erase(users[fd]);
							users.erase(fd);
							operators.erase(fd);

					    }



					}


					

					else if(buf.substr(0,5).compare("LIST\n") == 0) {

						int count = server_channels.size();
						int msize;
						char message[100];
						msize=sprintf(message, "There are currently %d channels.\n", count);
						send(fd, message, msize,0);
						for (int i=0; i < count; i++)
						{
							
							msize=sprintf(message, "* %s\n", server_channels[i].c_str());	
							send(fd, message, msize,0);
							
							
						}
						
						
						
					}

					else if(buf.substr(0,6).compare("LIST #") == 0) {

						
						string name = buf.substr(6);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end());
						int count = server_channels.size();
						int msize;
						
						if (user_channels.find(name) == user_channels.end())
						{
							
							char message[100];
							msize=sprintf(message, "There are currently %d channels.\n", count);
							send(fd, message, msize,0);
							for (int i=0; i < count; i++)
							{
								
								msize=sprintf(message, "* %s\n", server_channels[i].c_str());	
								send(fd, message, msize,0);
								
							
							}
						}
						else 
						{
							char message[100];
							msize=sprintf(message, "There are currently %lu users.\n", user_channels[name].size());
							send(fd, message, msize,0);
							for (int i=0; i < user_channels[name].size(); i++)
							{
								
								msize=sprintf(message, "* %s\n", users[user_channels[name][i]].c_str());	
								send(fd, message, msize,0);
								
							
							}

						

						}
						
						
						
					}

					else if(buf.substr(0,6).compare("JOIN #") == 0) {

						
						string name = buf.substr(6);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end());
						if (name.length() > 19) {
							send(fd, "Invalid channel name\n", sizeof("Invalid channel name\n"),0); 
							bzero(buffer,1024);
							continue;
						}

						if(!regex(name)) {
							send(fd, "Invalid channel name\n", sizeof("Invalid channel name\n"),0); 
							bzero(buffer,1024);
							continue;							
						}

						if (user_channels.find(name) == user_channels.end())
						{
							
							server_channels.push_back(name);
							vector<int> newvector;
							newvector.push_back(fd);
							user_channels.insert(pair<string, vector<int> >(name,newvector));
							char message[100];
							int msize=sprintf(message, "Joined channel #%s\n", name.c_str());
							send(fd, message, msize,0);
						}
						
						else
						{
							if (find(user_channels[name].begin(), user_channels[name].end(), fd) ==user_channels[name].end())
							{
								for (int i=0; i < user_channels[name].size(); i++)
								{
									char message[100];
									int msize=sprintf(message, "#%s> %s joined the channel.\n",name.c_str(), users[fd].c_str() );
									send(user_channels[name][i],message, msize,0);
								}

								user_channels[name].push_back(fd);
								char message2[100];
								int msize2=sprintf(message2, "Joined channel #%s\n", name.c_str());
								send(fd, message2, msize2,0);	
							}
							
							else{continue;}

							

						}
							
						
						
					}


					else if(buf.substr(0,6).compare("PART #") == 0) {
					
						string name = buf.substr(6);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end());
						if (user_channels.find(name) == user_channels.end())
						{
							char message[100];
							int msize=sprintf(message, "You are not currently in #%s.\n",name.c_str());
							send(fd, message, msize,0);	
							continue; //channel does not exist
						}
						
						else
						{
							if (find(user_channels[name].begin(), user_channels[name].end(), fd) ==user_channels[name].end())
							{
								continue; //not in the channel, silent ignore
							}
							
							else //channel exists and this fd is in it
							{
								
								for (int i=0; i <  user_channels[name].size(); i++)
								{
									char message[100];
									int msize=sprintf(message, "#%s> %s left the channel.\n",name.c_str(), users[fd].c_str() );
									send(user_channels[name][i],message, msize,0);

								}

								vector<int>::iterator it;
								it = find(user_channels[name].begin(), user_channels[name].end(), fd);
								user_channels[name].erase(it);
							}

						}
						
						
						
						
					}



					else if(buf.substr(0,5).compare("PART\n") == 0) {
					
						map<string,vector<int> >::iterator mit;
						for (mit=user_channels.begin(); mit != user_channels.end(); mit++)
						{	
							
							if(find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd)!=user_channels[mit->first].end())
							{


								for (int i=0; i <  user_channels[mit->first].size(); i++)
								{
								char message[100];
								int msize=sprintf(message, "#%s> %s left the channel.\n",(mit->first).c_str(), users[fd].c_str());
								send(user_channels[mit->first][i],message, msize,0);

								}

								vector<int>::iterator it;
								it = find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd);
								user_channels[mit->first].erase(it);

							}

						}
						
						
						
						
						
					}


					else if(buf.substr(0,9).compare("OPERATOR ") == 0) {
						
						if (!ispassword) {continue;} //no password provided, so we can't promote people
						string pass = buf.substr(9);
						pass.erase(remove(pass.begin(), pass.end(), '\n'), pass.end());
						if (pass == password)
						{

							operators[fd]=true;
							send(fd,"OPERATOR status bestowed.\n", sizeof("OPERATOR status bestowed.\n"),0);
							continue;
						}
						
						send(fd,"Invalid OPERATOR command.\n", sizeof("Invalid OPERATOR command.\n"),0);
						
						
						
					}

					else if(buf.substr(0,6).compare("KICK #") == 0)
					{	

						if (operators[fd]==false) {continue;} //not an operator, can't do anything
						string params = buf.substr(6);
						
						size_t found = params.find(' ');
						if (found == string::npos) {continue;} //THere is no space separating channel and name, what do we do here?????
						
						string channel = params.substr(0,found);
						string name = params.substr(found+1);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end());
					
						
						if (user_channels.find(channel) == user_channels.end())
						{
							continue; //channel does not exist
						}

						if (nametofd.find(name) == nametofd.end())
						{
							continue; //username does not exist
						}
						
						int fd_kick=nametofd[name];
						if(find(user_channels[channel].begin(), user_channels[channel].end(), fd_kick)!=user_channels[channel].end())
						{
							for (int i=0; i <  user_channels[channel].size(); i++)
							{
								char message[100];
								int msize=sprintf(message, "#%s> %s has been kicked from the channel.\n",channel.c_str(), name.c_str() );
								send(user_channels[channel][i],message, msize,0);

							}

								vector<int>::iterator it;
								it = find(user_channels[channel].begin(), user_channels[channel].end(), fd_kick);
								user_channels[channel].erase(it);
						}

						else {continue;} //user is not in that channel, sned an error or something?
						

					}


					else if(buf.substr(0,9).compare("PRIVMSG #") == 0) {
						string params = buf.substr(9);

						size_t found = params.find(" ");
						if(found == string::npos) {continue;}

						string channel = params.substr(0,found);
						string message = params.substr(found+1);

						if(message.length() > 512) {
							send(fd,"Message too long.\n",sizeof("Message too long.\n"),0);
							continue;
						}

						if(user_channels.find(channel) == user_channels.end()) {
							continue; // channel does not exist
						}

						if(find(user_channels[channel].begin(), user_channels[channel].end(), fd) != user_channels[channel].end()) {

							for(int i = 0; i < user_channels[channel].size(); i++) {
								char mssg[message.length() + channel.length() + users[fd].length() + 5];
								int msize = sprintf(mssg, "#%s> %s: %s", channel.c_str(), users[fd].c_str(), message.c_str());
								send(user_channels[channel][i],mssg,msize,0);
							}
						}
						else {continue;} // user is not in channel

					}

					else if(buf.substr(0,8).compare("PRIVMSG ") == 0) {
						string params = buf.substr(8);

						size_t found = params.find(" ");
						if(found == string::npos) {continue;}

						string user = params.substr(0,found);
						string message = params.substr(found+1);

						if(message.length() > 512) {
							send(fd,"Message too long.\n",sizeof("Message too long.\n"),0);
							continue;							
						}

						if(nametofd.find(user) == nametofd.end()) {
							continue; // user does not exist
						}

						int user_fd = nametofd[user];
						
						char mssg[users[fd].length() + message.length() + 2];
						int msize = sprintf(mssg, "%s> %s",users[fd].c_str(), message.c_str());
						send(user_fd,mssg,msize,0);

					}

					else if(buf.substr(0,5).compare("QUIT\n") == 0) {


						map<string,vector<int> >::iterator mit;
						for (mit=user_channels.begin(); mit != user_channels.end(); mit++)
						{	
							
							if(find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd)!=user_channels[mit->first].end())
							{


								for (int i=0; i <  user_channels[mit->first].size(); i++)
								{
									if(user_channels[mit->first][i] == fd) {continue;}
									char message[100];
									int msize=sprintf(message, "#%s> %s left the channel.\n",(mit->first).c_str(), users[fd].c_str());
									send(user_channels[mit->first][i],message, msize,0);

								}

								vector<int>::iterator it;
								it = find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd);
								user_channels[mit->first].erase(it);

							}

						}	

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

						nametofd.erase(users[fd]);
						users.erase(fd);
						operators.erase(fd);											

					}


				}

			}
	
  
		}


	}



}
