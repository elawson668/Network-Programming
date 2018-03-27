#include "User.h"
#include <string>

User::User(int fdesc, std::string n) {
	fd = fdesc;
	name = n;
}