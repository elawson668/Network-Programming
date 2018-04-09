README.txt ASSIGNMENT #: IRC CHAT SERVER
AUTHORS: Matthew Mohr, Eric Lawson


Implementation details:

Starting server: 
When starting the serrver, you can either run the executable or supply a single command line argument in addtion to the executable called "--opt-pass=<password>". Calling this as the first command line argument will set the password for operators if the passwrod follows the regex outlined in the assingment specifications. Calling "--opt-pass=" will inform the user that this is not a valid password, as well as any other passwords that do not adhere to the regex. In these cases, the server will not start. Upon successfully starting the server, the port number will be output to standard output.


The code supports the following list of commands. All inputs from the netcat termnal, in order to be more user friendly, MUST be terminated by using thr newline character. For convenince, valid inputs are listed as follows, along with expected behavior:

"USER <username>\n"
--Must be first command user inputs on new connection. If it is not, th connection is terminated. Entering "USER \n" without a valid username, entering a username with more than 20 characters, or entering a username that doesn't follow the regex outlined by the assingment  will result in a message indicating an invalid username followed by connection termination. Calling USER after a valid USER command has been sent will just result in the clinet receiving a message indiating that it is an invalid command. 

"LIST\n" and "LIST #<channel>\n" 
--"LIST\n" will simply list the count of alll channels and what they are. "LIST \n" will be recognized as an invalid command. "LIST <channel>\n" without the '#' sign preceding the channel name will be recognized as invalid. "LIST #<channel>\n" with a channel that does not exist will simply print out all channels. Calling "LIST #<channel>\n" will output all users on a valid channel.

"JOIN #<channel>\n"
--"JOIN \n" and "JOIN <channel name>\n" without the '#' are recognized as invalid commands. "JOIN #\n" and other channel names taht do not adhere to the regex outlined in the assignment spcifications will report tha the cahnnel name is invalid ot the user. All valid commands will inform the user that they have joined a channel, and any other users present on the channel are notified.

"PART\n" and "PART #<channelname>\n"
--"PART \n" and "PART <channel name without #>\n" ae recognized as invalid commands. A valid call to "PART\n" will remove the user form all channels they are currently part of and will inform them of the channels they have left. All other users will see that they left the channel as well from a similar message. Calling "PART #<channelname>\n" on a channel that the user is not part of will tell the user they are not part of the channel. Calling PART on a channel that does not exist will send a message to the user indicating that the channel doesnot exist. A valid call to "PART #<channelname>\n" will notify all users of the channel that the user is leaving before removing the user from the channel.

"OPERATOR <password>\n"
--If the server was not started with a password, any call consisting of the string starting with "OPERATOR " will inform the user that no one can be promoted to an operator status. IF a password was supplied, "OPERATOR \n" and "OPERATOR <invalid password>\n" will inform the user that it is an invalid OPERATOR command. Calling OPERATOR with the correct password will bestow operator status.

"KICK #<channel> <username>\n"
--IF a user is not an operator, the user will be informed of this and nothing will happen. Channel name and username must be separated by a space and valid channel names must begin with #, otherwise the command will be treated as invalid. If the channel or the user does not exist, the server will infrom the user that these entities do not exist. If the current user is not an operator, a message will be sent informing the user of this. Calling "KICK #<channel> \n" will infrom the user that no such user exists if the channel is valid, wheras "KICK #<channel>\n" is an invalid command. Succesful KICK calls will inform all person in the channel that the user was kicked. A user can KICK any person from any channel, even if the user is not currently part of said channel. Attempts to kick a user from a channel they are not part of will result in an error message. 

"PRIVMSG <username> <message>\n" and "PRIVMSG #<channel> <message>\n"
--If a space is not found between the target of the message, the command is treated as invalid. A user must be part of a channel to send a meesage, otherwise they receive an error message. "PRIVMSG \n" is invalid, whereas with two spaces "PRIVMSG  \n" will send that the user does not. If a channel does not exist or a user does not exist, the command will inform the user that their target does not exist with a message. Messages must be no more than 512 characters, otherwise the command is treated as invalid. If the user option is taken and the user exists, then the user will revceive the message/ "PRIVMSG <valid user> \n" will send a newline to the target user, and the same behavior occurs with "PRIVMSG <valid channel> \n" for the channel as the newline character is treated as part of the message. 

"QUIT\n"
--Must be appneded by newline. Removes usrs from all channels and shuts down their conection. If a user exists with crtl-c instead (or any other form of abrupt termination on the client side), the same behvaior is followed. 



Issues encountered:
We had trouble deciding how to pars einput commands, so we did an approach where we read the leading expected characters in (i.e. reading it in as "LIST #" rather than parsing "LIST"  and everytjing the follows). Aside from complexity of dealing with c++ and c strings together, we really had no issues and this was a straightforward assignment.


Time spent:
3 to 4 hours a day for the course of a week


Work Breakdown: 
Matt:
-Intial server code with select,implemented all commands except for PRIVMSG and QUIT and created a way to send error message leveraging both c and c++ strings
ERIC:
-Implemented PRIVMSG and QUIT behavior for abrupt termination and connection closing, handled edge cases for imporper inputs on commands, implemented regex beahvior
