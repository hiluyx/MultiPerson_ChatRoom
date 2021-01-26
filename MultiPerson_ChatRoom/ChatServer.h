#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <vector>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")

#include "Thread.h"
#include "Client.h"

static const PCSTR DEFAULT_PORT = "12705";

class SubMessageRouter
{
private:
	void* execHandle;
	bool IsExec;
	static unsigned WINAPI onMessageExecuteThread(void* param);
	void SendAll(string& s);
	void SendOne(char* d_username, string& s);
public:
	SubMessageRouter();
	bool DoWork();
	void StopWork();
};

class ChatServer: public Thread
{
private:
	SOCKET                    listenSocket;
	bool                          IsServerRunning;
	SubMessageRouter  router;
	static bool loginHandler(SOCKET& newClient, char *logincs, int len, int& uIndex);
protected:
	void stopServer();
	bool init_server();
public:
	ChatServer();
	bool ServerIsRunning();
	void DoWork();
	int CleanDeadClient();
	~ChatServer()
	{
		stopServer();
	}
}; 

#endif