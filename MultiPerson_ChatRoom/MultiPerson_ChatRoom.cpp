#include "ChatServer.h"

using namespace std;

extern ChatServer server;
extern vector<User> users;

void  read_userdata_fromDisk(const char* file);
int main() {
	read_userdata_fromDisk("_user_data.txt");
	server.Start();
	server.WaitForWork();
	return 0;
}

void  read_userdata_fromDisk(const char* file)
{
	int index = 0;
	ifstream userdata_infile(file);
	if (!userdata_infile.is_open())
	{
		userdata_infile.close();
		throw new exception("userdata_infile open fail!");
	}
	else {
		char name_buf[10], pass_buf[10];
		while (!userdata_infile.eof())
		{
			userdata_infile >> name_buf >> pass_buf;
			char* name, * pass;
			name = new char[10];
			pass = new char[10];
			strcpy(name, name_buf);
			strcpy(pass, pass_buf);
			cout <<"add user:"<< name << " pass:" << pass << endl;
			users.push_back(User(name, pass));
		}
		userdata_infile.close();
	}
}
