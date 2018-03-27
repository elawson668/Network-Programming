#ifndef __user_h_
#define __user_h_

class User {
public:

	int fd;
	std::string name;
	bool op = false
	bool first = true;

	User(int fdesc, std::string n);
}

#endif