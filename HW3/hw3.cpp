
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


#define LONG_TIME 100000000 //timeout value

int client_fds[100]; //array for clients, we can store at most 100 clients
char buffer[1024];   //incoming character buffer for data

map<string,vector<int> > user_channels; //names of channels, value is array of fds in that channel 
map<int,string> users; //usernames are keys, fd for that user is value 
map<string, int> nametofd; //inverse dicitonary of users
map<int,bool> operators; //fd is the fd for a user, boolean is if they are an operator or not
vector<string> server_channels; //list of all channels
string password; //password that the server starts on
bool ispassword = false; //whehter there is a password or not


static volatile int timeOut = LONG_TIME; //timeout for select



//Function that determined whther an input string adheres to the regex
//Parameters: 
//word: the input word we are checking, either the username or the channel name (without the '#')
//Return value: false if it does not adhere to regex, true otherwise 
bool regex(string word) {

	int n = word.length();
	char w[n];
	strcpy(w,word.c_str()); //take c++ string and analyze it cahracter by character

	if(!isalpha(w[0])) return false; //first character must be alpahbetical

	for(int i = 1; i < n; i++) { //check to make sure rest of characters are alphanumeric or undersocre
		if(!isalpha(w[i]) && !isdigit(w[i]) && w[i] != '_') return false;
	}

	return true;
}


//main method
int main(int argc, char* argv[])
{

    if(argc == 2) //if we have 2 command line arguments, check to make sure it is he operator flag
    {
	string flag = argv[1]; //read the other flag as a string
	size_t equals = flag.find_first_of('='); //find the equals sign in the argument

	if (flag.substr(0,equals+1) == "--opt-pass=") //chekc to make sure it is the flag we want
	{
		password = flag.substr(equals+1); //position of equals sign plus 1 is where the passwor starts, no given password just returns an empty string

		if(!regex(password)) { //if the password does not match the regex, don't start server
			cout << "Startup failed - invalid password.\n";
			return 0;
		}

		ispassword=true; //all is well, we have a validpassword
	}



    }
    int listener_socket = socket( AF_INET, SOCK_STREAM, 0 ); //listener socket

	if ( listener_socket < 0 )
	{
	  perror( "socket()" );
	  exit( EXIT_FAILURE );
	}


	struct sockaddr_in server; //server and client basic info
	struct sockaddr_in client;
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY; 
	server.sin_port = htons(0); //randomly assigned os port number to start the server
	int len = sizeof( server );

	if ( bind( listener_socket, (struct sockaddr *)&server, len ) < 0 ) //bind listener socket to the port number
	{
	  perror( "bind()" );
	  exit( EXIT_FAILURE );
	}

	if (getsockname (listener_socket, (struct sockaddr *) &server, (socklen_t *) &len) < 0) //sget socket information so we can print the port number
	{
		perror("ERROR: getsockname() failed\n"); 
		return EXIT_FAILURE;
	}

	
	
	int tport = ntohs(server.sin_port); //print out what port number we binded to
	printf("%d\n", tport);
	listen( listener_socket, 5 ); //we are now listeneing on listener socket
	int clientlen = sizeof( client );
	struct timeval tv;
	tv.tv_sec = timeOut; //set up timeout struct for select
	tv.tv_usec = 0;


	int index=0; //index of last client we currntly have in array
	while ( 1 ) //loop forever
  	{
		fd_set readfds; //set of file descriptorsto wathc
		FD_ZERO( &readfds );
    	FD_SET( listener_socket, &readfds ); //add listener to selet set
		for (int i = 0 ; i < index ; i++ ) //loop up to max number of cleints and add thee file descriptors as well
    	{
      		FD_SET( client_fds[ i ], &readfds ); 
        }

		int result = select(FD_SETSIZE, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv); //select to monitor htese fds

		if (result ==0) {continue;} //no activity keep going forever

		if (FD_ISSET(listener_socket, &readfds)) //we have activity on listener
		{			

			int newconnection = accept(listener_socket, (struct sockaddr *) &client, (socklen_t *) &clientlen); //accept new connection and ad it to the client array
			client_fds[ index++ ] = newconnection;
		}

		for ( int j = 0 ; j < index ; j++ ) //loop over all clients, do something if one of the clients has activity, check what it is
		{
			int fd = client_fds[ j ];
		    if ( FD_ISSET( fd, &readfds ) )
			{
				int incoming_bytes=recv(fd, buffer, 1024, 0); //see how many bytes we recieved
				
				if (incoming_bytes == 0) //no bytes means closed conneciton
				{
					map<string,vector<int> >::iterator mit; //iterator over all channels, check each channel one at a time to see if current fd is there
						for (mit=user_channels.begin(); mit != user_channels.end(); mit++) 
						{	
							//the user is currently in this channel, remove them
							if(find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd)!=user_channels[mit->first].end())
							{
								//before they are removed from the channel, sned a message to channel saying that they left
							for (int i=0; i <  user_channels[mit->first].size(); i++)
							{
							char message[100]; //user name and current channel name
							int msize=sprintf(message, "#%s> %s left the channel.\n",(mit->first).c_str(), users[fd].c_str());
							send(user_channels[mit->first][i],message, msize,0); //send to all fds in channel

							}

							vector<int>::iterator it; //remove fd from the channel
							it = find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd);
							user_channels[mit->first].erase(it);

						}

					}
					close( fd ); //close the connection
					for ( int a = 0 ; a < index ; a++ ) //find the descriptor that is closing
					{
						if ( fd == client_fds[ a ] ) //we found it, copy everything that connect after it
						{
						     
							for ( int b = a ; b < index - 1 ; b++ ) //go up to the last index we have -1, we are copying the i+1th position to 													postion i
						    {
								client_fds[ b ] = client_fds[ b + 1 ];
						    }
						    index--; //one less client means max client number is now one less
						    break;     
					   	}
					}
					
					nametofd.erase(users[fd]); //erase this fdfrom all other maps, user is no longer present
					users.erase(fd);
					operators.erase(fd);
						

				}

				else //we got more than 0 bytes, so we have data to look at
				{

					
					buffer[incoming_bytes]='\0'; //null terminate input for c++ string comparison

					string buf = buffer; //make buffer a c+ string

					if (users.find(fd) == users.end()) { //if the current fd does not have an entry, this is a new connection, first command must be user


						if(buf.substr(0,5).compare("USER ") == 0) { //command is user

							string name = buf.substr(5);
							name.erase(remove(name.begin(), name.end(), '\n'), name.end());
							if(name.length() > 20) { //username is too long, return invalid username
								send(fd, "Invalid username.\n", sizeof("Invalid username.\n"),0);
								bzero(buffer,1024);
							
								close( fd ); //invalid username means we kill the conneciton
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

								nametofd.erase(users[fd]); //erase current fd from all entries
								users.erase(fd);
								operators.erase(fd);

								continue; //keep looping
							}
							//check to see if the name adheres to the regex, if not this will be true and we kill the conneciton
							if(!regex(name)) {
								send(fd, "Invalid username.\n", sizeof("Invalid username.\n"),0);
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

								nametofd.erase(users[fd]); //remove current fd and username from all enntries
								users.erase(fd);
								operators.erase(fd);

								continue; //keep looping

							}

							if (nametofd.find(name) != nametofd.end()) //the username is already in the dcitionary, cannot take it twice, kill connection after informing user
							{
							send(fd, "Username already taken, terminating initial connection.\n", 
							sizeof("Username already taken, terminating initial connection.\n"),0);
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
							nametofd.erase(users[fd]); //erase user from all disctionaries and then continue looping
							users.erase(fd);
							operators.erase(fd);
							continue;
							}
							//if we made it tis far without continuing, everything is in order, so we can add the username and fd to revelvant dictionaries

							users.insert(pair<int,string>(fd,name));
							operators.insert(pair<int,bool>(fd,false));
							nametofd.insert(pair<string,int>(name,fd)); //after user is logged in on all dicitonaries, send welcome message
							char message[100];
							int msize=sprintf(message, "Welcome, %s\n", name.c_str());
							send(fd, message, msize,0);
						
						}

						else { //the user enters something else beisdes user for their first time, kill connection
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
							operators.erase(fd); //erase user info if it exists

					    }



					}


					

					else if(buf.substr(0,5).compare("LIST\n") == 0) { //LIST with no channel, just print out all channels

						int count = server_channels.size();
						int msize;
						char message[100]; //send number of channels to a person
						msize=sprintf(message, "There are currently %d channels.\n", count);
						send(fd, message, msize,0);
						for (int i=0; i < count; i++) //loop through all channels and print their names
						{
							
							msize=sprintf(message, "* %s\n", server_channels[i].c_str());	
							send(fd, message, msize,0);
							
							
						}
						
						
						
					}

					else if(buf.substr(0,6).compare("LIST #") == 0) {//list with a '#' indicates that there is a channel we wantto see

						
						string name = buf.substr(6);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end()); //erase trailing newline from input if it exists
						int count = server_channels.size(); //total number of channels
						int msize;
						
						if (user_channels.find(name) == user_channels.end()) //we cant find the channel name because it doens texist, print out channel names
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
						else //we did find the channel, print out who is in it
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

					else if(buf.substr(0,6).compare("JOIN #") == 0) {//JONI command, '#' is not optional, invalid otherwise

						
						string name = buf.substr(6);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end()); //remove trailing newline
						if (name.length() > 19) { //channel name is too long, so continue
							send(fd, "Invalid channel name.\n", sizeof("Invalid channel name.\n"),0); 
							bzero(buffer,1024);
							continue;
						}

						if(!regex(name)) {//channel name does no adhere to regex, invalid channel name
							send(fd, "Invalid channel name.\n", sizeof("Invalid channel name.\n"),0); 
							bzero(buffer,1024);
							continue;							
						}

						if (user_channels.find(name) == user_channels.end()) //channel doe snot exist yet, so add it in, add current user to that channel
						{
							
							server_channels.push_back(name);
							vector<int> newvector;
							newvector.push_back(fd);
							user_channels.insert(pair<string, vector<int> >(name,newvector));
							char message[100];
							int msize=sprintf(message, "Joined channel #%s\n", name.c_str());
							send(fd, message, msize,0);
						}
						
						else //channel does exist, we need to add user to it and inform everyone on the channel that a user joined
						{
							if (find(user_channels[name].begin(), user_channels[name].end(), fd) ==user_channels[name].end()) //user is not on this channel
							{
								for (int i=0; i < user_channels[name].size(); i++) //inform everyone that the user is in channel
								{
									char message[100];
									int msize=sprintf(message, "#%s> %s joined the channel.\n",name.c_str(), users[fd].c_str() );
									send(user_channels[name][i],message, msize,0);
								}

								user_channels[name].push_back(fd); //tell user that they joined the channel
								char message2[100];
								int msize2=sprintf(message2, "Joined channel #%s\n", name.c_str());
								send(fd, message2, msize2,0);	
							}
							
							//user is part of this channel, send a message indicating this
							else{send(fd, "You are already a part of this channel.\n", 
								sizeof("You are already a part of this channel.\n"),0); continue;}

							

						}
							
						
						
					}


					else if(buf.substr(0,6).compare("PART #") == 0) {//PART for a single channel
					
						string name = buf.substr(6);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end()); //erase trailing newline
						if (user_channels.find(name) == user_channels.end())
						{
							
							send(fd, "This channel does not exist.\n", sizeof("This channel does not exist.\n"),0);	
							continue; //channel does not exist
						}
						
						else
						{
							if (find(user_channels[name].begin(), user_channels[name].end(), fd) ==user_channels[name].end())
							{
								char message[100];
								int msize=sprintf(message, "You are not currently in #%s.\n",name.c_str());
								send(fd, message, msize,0);								
								continue; //not in the channel, inform the user and continue
							}
							
							else //channel exists and this fd is in it
							{
								
								for (int i=0; i <  user_channels[name].size(); i++) //loop through all fds in the channel and inform everyone in it that the user left
								{
									char message[100];
									int msize=sprintf(message, "#%s> %s left the channel.\n",name.c_str(), users[fd].c_str() );
									send(user_channels[name][i],message, msize,0);

								}

								vector<int>::iterator it; //once users are infromed, remove user
								it = find(user_channels[name].begin(), user_channels[name].end(), fd);
								user_channels[name].erase(it);
							}

						}
						
						
						
						
					}



					else if(buf.substr(0,5).compare("PART\n") == 0) { //PART with no arguments, remove user from all channels
					
						map<string,vector<int> >::iterator mit; //iterator for all channels
						for (mit=user_channels.begin(); mit != user_channels.end(); mit++) //loop through all channels one at a time
						{	
							
							if(find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd)!=user_channels[mit->first].end()) //if user is in channel
							{


								for (int i=0; i <  user_channels[mit->first].size(); i++) //loop through all users of channel, inform them that someone has left
								{
								char message[100];
								int msize=sprintf(message, "#%s> %s left the channel.\n",(mit->first).c_str(), users[fd].c_str());
								send(user_channels[mit->first][i],message, msize,0);

								}

								vector<int>::iterator it; //remove user fd from the channel
								it = find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd);
								user_channels[mit->first].erase(it);

							}

						}
						
						
						
						
						
					}


					else if(buf.substr(0,9).compare("OPERATOR ") == 0) {//OPERATOR COMMAND with a space
						
						if (!ispassword) //server did not start with a password, keep looping
						{	
							send(fd, "Server did not start with password. No operators allowed.\n",
							sizeof("Server did not start with password. No operators allowed.\n"),0);
							continue;
						} //no password provided, so we can't promote people

						string pass = buf.substr(9);
						pass.erase(remove(pass.begin(), pass.end(), '\n'), pass.end());//password was given, remove trailing newline
						if (pass == password) //if it matched the password, promote them to operator status
						{

							operators[fd]=true;
							send(fd,"OPERATOR status bestowed.\n", sizeof("OPERATOR status bestowed.\n"),0);
							continue;
						}
						
						send(fd,"Invalid OPERATOR command.\n", sizeof("Invalid OPERATOR command.\n"),0); //password was wrong, ignore it
						
						
						
					}

					else if(buf.substr(0,6).compare("KICK #") == 0) //KICK from channel command
					{	

						if (operators[fd]==false) { //if current user is not operator, we cant kick people
						
						send(fd, "Current user is not an operator.\n",
						sizeof("Current user is not an operator.\n"),0);
						continue;} //not an operator, can't do , keep loooing
						string params = buf.substr(6);
						
						size_t found = params.find(' ');
						if (found == string::npos) {send(fd, "Invalid command.\n", sizeof("Invalid command.\n"), 0); continue;} //user did not input another space specifying a user, keep looping
						
						string channel = params.substr(0,found); //user input both a channel name and a username
						string name = params.substr(found+1);
						name.erase(remove(name.begin(), name.end(), '\n'), name.end()); //remove newline from the end of the string for the name
					
						
						if (user_channels.find(channel) == user_channels.end()) 
						{
							send(fd, "This channel does not exist.\n", sizeof("This channel does not exist.\n"),0);	
							continue; //channel does not exist
						}

						if (nametofd.find(name) == nametofd.end())
						{
							send(fd, "This user does not exist.\n", sizeof("This user does not exist.\n"),0);	
							continue; //username does not exist
						}
						
						int fd_kick=nametofd[name];//find the fd of the username we wnat to get rid of
						if(find(user_channels[channel].begin(), user_channels[channel].end(), fd_kick)!=user_channels[channel].end()) //we found the fd in the channel we were looking for
						{
							for (int i=0; i <  user_channels[channel].size(); i++) //inform all users that this person was kicked
							{
								char message[100];
								int msize=sprintf(message, "#%s> %s has been kicked from the channel.\n",channel.c_str(), name.c_str() );
								send(user_channels[channel][i],message, msize,0);

							}

								vector<int>::iterator it; //remove user from channel
								it = find(user_channels[channel].begin(), user_channels[channel].end(), fd_kick);
								user_channels[channel].erase(it);
						}

						else {send(fd, "User is not part of this channel, cannot kick.\n", 
							sizeof("User is not part of this channel, cannot kick.\n"),0);
							continue;} //user is not in that channel
						

					}


					else if(buf.substr(0,9).compare("PRIVMSG #") == 0) {//PRIVMSG to channel option
						string params = buf.substr(9);

						size_t found = params.find(" ");//find space separating channel and message
						if(found == string::npos) {send(fd, "Invalid command.\n", sizeof("Invalid command.\n"), 0); continue;}//no space, no message

						string channel = params.substr(0,found); //store channel name, store message name 
						string message = params.substr(found+1);

						if(message.length() > 513) {//513 is 512 including the newline character, anything longer than 512 plus newline is too long
							send(fd,"Message too long.\n",sizeof("Message too long.\n"),0);
							continue;
						}

						if(user_channels.find(channel) == user_channels.end()) {
							send(fd, "This channel does not exist.\n", sizeof("This channel does not exist.\n"),0);	
							continue; // channel does not exist
						}

						if(find(user_channels[channel].begin(), user_channels[channel].end(), fd) != user_channels[channel].end()) {//user is in channel

							for(int i = 0; i < user_channels[channel].size(); i++) {
								char mssg[message.length() + channel.length() + users[fd].length() + 5];//space for extra characters, channel and meesage
								int msize = sprintf(mssg, "#%s> %s: %s", channel.c_str(), users[fd].c_str(), message.c_str());
								send(user_channels[channel][i],mssg,msize,0); //send message
							}
						}
						else {send(fd, "You are not part of this channel, cannot send message.\n", 
							sizeof("You are not part of this channel, cannot send message.\n"),0);
							continue;} // user is not in channel

					}

					else if(buf.substr(0,8).compare("PRIVMSG ") == 0) {//PRIVMSG was not a channel based thing, must be private to a user
						string params = buf.substr(8);

						size_t found = params.find(" ");
						if(found == string::npos) {send(fd, "Invalid command.\n", sizeof("Invalid command.\n"), 0); continue;}//no space, no message

						string user = params.substr(0,found);
						string message = params.substr(found+1);

						if(message.length() > 513) {//newline plus 512 characters, anything more htan that is too long
							send(fd,"Message too long.\n",sizeof("Message too long.\n"),0);
							continue;							
						}

						if(nametofd.find(user) == nametofd.end()) {
							send(fd, "This user does not exist.\n", sizeof("This user does not exist.\n"),0);
							continue; // user does not exist
						}

						int user_fd = nametofd[user];//fd of the user we want to send to
						
						char mssg[users[fd].length() + message.length() + 2];//send the message to the user's fd
						int msize = sprintf(mssg, "%s> %s",users[fd].c_str(), message.c_str());
						send(user_fd,mssg,msize,0);

					}

					else if(buf.substr(0,5).compare("QUIT\n") == 0) {//QUIT, takes no arguments


						map<string,vector<int> >::iterator mit; //iterator for channel map
						for (mit=user_channels.begin(); mit != user_channels.end(); mit++)
						{	
							//for each channel, check if user is in there
							if(find(user_channels[mit->first].begin(), user_channels[mit->first].end(), fd)!=user_channels[mit->first].end())
							{


								for (int i=0; i <  user_channels[mit->first].size(); i++) //user is in there, inform everyone that the user is leaving
								{
									if(user_channels[mit->first][i] == fd) {continue;}
									char message[100];
									int msize=sprintf(message, "#%s> %s left the channel.\n",(mit->first).c_str(), users[fd].c_str());
									send(user_channels[mit->first][i],message, msize,0);

								}

								vector<int>::iterator it; //remove user form channel
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

						nametofd.erase(users[fd]); //after close, remove every trace of the user from themaps
						users.erase(fd);
						operators.erase(fd);											

					}


					else{//command is something other than options, it is invalid

						send(fd, "Invalid command.\n", sizeof("Invalid command.\n"), 0);
						
					}


				}

			}
	
  
		}


	}



}
