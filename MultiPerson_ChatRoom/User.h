#pragma once
#include <windows.h>

class User
{
private:
	char*           password;
	char*           name;
public:
	User(char* _name, char* _password)
	{
		name = _name;
		password = _password;
	}

	bool login_verification(const char* pass);
	inline char* getname()
	{
		return name;
	}
};

