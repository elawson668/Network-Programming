HW2 README: ROCK PAPER SCISSORS TCP SERVER
Authors: Matthew Mohr annd Eric Lawson


PROGRAM DESCRIPTION:
This code creates a TCP server and registers it through Zeroconf using the Zeroconf functon from the textbook shown in class. The server chooses a random OS assigned port numbe rthat can e discovered using the commands (server is named mohrm):


Bonjour: dns-sd -L mohrm _rps._tcp local 
Avahi: avahi-browse -d local _rps._tcp --resolve


ZEROCONF:
Additionally, the code outputs the port number on the terminal where the server is running itself to demonstrate that it is bound to a port number on the OS. Upon successful registration with Zeroconf services, the server will output a meesage containing the string "REGISTER mohrm", indicating that the service was successfully registered to Zeroconf.

THE GAME:
-Upon connecting, a cloent will immediately receive the message "What is your name?", indicating that a successful connection has been made. 
The client will continue to enter their name until a valid name is entered by the user. 
Since we are using the netcat terminal for connections and we wish for the application to be user friendly, the string for a client name must be appended with the newline character. 
(i.e. If my name is matt, "matt\n" will be sent ot he server). The same format is also true for choices for the game (ie. rock\n, paper\n, sicssors\n). Names and choices are not case sensitive and are handled appropirately. Game computation outputs the player names and game choices using uppercase characters when printing out the game result.

FAULT TOLERANCE:
-If a client disconnects at any of the three following points:
	--After connecting and not inputting a name;
	--After inputing a valid name but not a game choice
	--After sbmitting both a valid name and a valid choice
Then the server handles this by erasing any data that the cleint entered and forcibly closes the client connection so that a new player cna enter the game. Disconnect behavior occurs when the client disconnects manually or whenever the client sends exactly 0 bytes of data to the server. Once disconnected, a new client can connect to the game and enter their coices. These disconnects are independent between clients, so one client disconnecting has no effect on the other client and that other cleints choices are preserved. A game is computed only when both of the two client connections have entered both a valid name and a valid game choice using the newline character for all entries. At this point, the game result is determined and sent to both clients, then the clients are disconnected for two more players to enter the game. 


Issues encountered: 
Zeroconf registration issues were not reaidly apparent and we limited our testing to the two browse commands above to monitor the connection. Additionally we dealt with segmentation faults on different perating systems a a result of not checking FD_ISSET on a file descriptor equal to -1, but this erorr has been fixed. for the case where two clients are already in game, the behavior for an adidtional external client attempting to connect is undefined. This server can support at most two client conenctions at a time. According to one post on Piazza and with thepermission of theinstructor, this case of additional conenctions occuring will not be tested and our code submission is fine with leaving this behavior as undefined. 

Time spent: Approximately 3-6 hours a day for about a weke for both parties. 


Group work breakdown:
Matt: Setup TCP server and Zeroconf registration, employed select mechanism for cleints and handling client connections, error checks on client inputs, testing using avahi
Eric: Implemented Game logic and computation of game state, formatted strings for optimal user input and output. Helped to implmnet clean disconnect for clients as well, tests using Bonjour


