Authors: Matthew Mohr and Eric Lawson
README.txt for Assignment 1: TFTP Server

Included Files:
Makefile
hw1finalversion.cpp
README.txt


Known Behavior:

Our server adheres to the design principles laid out in RFC 1350. The server supports all 5 types of packet transmissions (RRQ, WRQ, DATA, ACK, and ERROR). 
Expeted and notable design choices concerning the  behavior of our application is documented as follows:

-The server will initially expect either a RRQ or a WRQ packet to be sen tby the client before doing any TFTP operations. If this is not the case, the user will be informed that an illegal operation has occurred via an error packet.
-Once received, a child process will spawn to handle the request. If it is aread request, the server process will wait to receive ACK packets from the user. If it's a WRQ, then the process will block waiting for data pakcets. Both receive calls are blocking but are monitored using timeouts.
-Timeout behavior is implemented using a combination of SIGALRM usong a custom handler and the signinterrupt() method. signinterrupt guarantees that if a recvfrom call times out, EINTR will be thrown as opposed to the function call simpyl restarting. Doing this allows us to check for when data has not been receieved for one second. If any amount of bytes is read in during that one second, regardless of whether the packet is well formed or not, the global timeout counter resets to 0. If an alarm happens 10 times, then the child process terminates and immeidately stops reading and or writing. Whatever amount of bytes was read into the client or was written by the client t the current directory that the program is inis all that will be there, leaving an incomplete file. To attempt a re-write of th efile, the user must delete the incomplete file first or else they will receive a "FILE NOT FOUND ERROR".
-The Sorcerror's apprentice bug is handled by checking that the block number of an incoming ACK packet is equal to the one we are expecting based on the number of the data packet we sent out. Any ACK packet with a lower block number gets ignored, but the alarm is reset because we did recieve data. 


Error Code Support: 
The following error codes are currently supported in these situations:


   1         File not found (oocurs when a read request cannot locate the file to read)
   2         Access violation (occurs on a read request if the client does not have permission to read the file)
   4         Illegal TFTP operation (sent if the first packet received is not a RRQ or a WRQ)
   5         Unknown transfer ID (sent if port number initally matched by the client does not match an incoming port number from a client during a later packet transsfer, this does not terminate the connection, all other error codes do this)
   6         File already exists (occurs on a WRQ if the file is already present on the server side, )
   