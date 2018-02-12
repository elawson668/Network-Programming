void handle_alarm(int signum);

void handle_error(int sd, struct sockaddr * client, socklen_t* length, uint16_t errcode, std::string message);

void handle_read_request(char* filename, int sd, struct sockaddr* client, socklen_t* length);

void handle_write_request(char* filename, struct sockaddr * client, socklen_t* length);
