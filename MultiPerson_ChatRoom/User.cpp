#include "User.h"

bool User::login_verification(const char* pass) 
{
	if (strlen(pass) <= 0 || strlen(password) != strlen(pass))
	{
		return false;
	}
	if (0 == strcmp(password, pass))
	{
		return true;
	}
	else
	{
		return false;
	}
}