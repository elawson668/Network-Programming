#include "dns_sd.h"

int isvalidname(char* name, int size);

int isvalidchoice(char* choice, int size);

void game(int s1, int s2, char* c1, char*c2, int sd1, int sd2);

void HandleEvents(DNSServiceRef serviceRef, int listen_socket);

static void
MyRegisterCallBack(DNSServiceRef service,
				   DNSServiceFlags flags,
				   DNSServiceErrorType errorCode,
				   const char * name,
				   const char * type,
				   const char * domain,
				   void * context);

static DNSServiceErrorType MyDNSServiceRegister(tport, listener_socket);

