#include "ChatServer.h"
#include "LoopQueue.h"

using namespace std;

SubMessageRouter router;
ChatServer server;
LoopQueue m_queue;
vector<User> users;
vector<Client*> clients;

ChatServer::ChatServer()
{
	// 先设置为无效链接，0xFFFFFFF
	listenSocket = INVALID_SOCKET;
	IsServerRunning = false;
}

bool ChatServer::ServerIsRunning()
{
	return IsServerRunning;
}

void ChatServer::DoWork()
{
	if ( !init_server()) return;
	int iResult = 1;
	IsServerRunning = true;
	router.DoWork();
	cout << "start server..." << endl;
	if (listen(listenSocket, 5) == SOCKET_ERROR)
	{
		cout << "listen fail" << endl;
		return;
	}
	char* loginChars = new char[50];
	while (IsServerRunning)
	{
		SOCKET clientSocket = accept(listenSocket, NULL, NULL);// 接受新的客户端连接
		if (clientSocket != INVALID_SOCKET)
		{
			cout << "新连接" << endl;
			CleanDeadClient();
			int userIndex = 0;
			if (loginHandler(clientSocket,loginChars,50, userIndex))// 连接成功、验证登录
			{
				Client* p_c = new Client(clientSocket, &users[userIndex]);
				cout << "登录成功，通知所有用户有人上线了" << endl;
				p_c->startRunning(); // 运行该客户端的接发线程
				clients.push_back(p_c);
				string loginMess;
				loginMess.append("N,");
				cout << loginMess << endl;
				m_queue.EnQueue(loginMess);
			}
			else
			{
				cout << "非法登录或超时" << endl;
				char loginFailRet[] = { "N,N" };
				send(clientSocket, loginFailRet, 3, 0); // 发送N,N
				continue;
			}
		}
	}
	delete[] loginChars;
}

int ChatServer::CleanDeadClient()
{
	int deadcount = 0;
	for (vector<Client*>::iterator iter = clients.begin(); iter != clients.end();)
	{
		if (!(*iter)->IsConnected())
		{
			cout << "delete a dead client" << endl;
			deadcount++;
			delete* iter;
			iter = clients.erase(iter);
		}
		else
		{
			iter++;
		}
	}
	return deadcount;
}

bool ChatServer::loginHandler(SOCKET& newClient, char* logincs, int len, int &uIndex)
{
	int recvMaxTime = 999;// 重试接受登录消息次数，超过时间视为登录失败。
	while (recvMaxTime > 0)
	{
		int irecv = recv(newClient, logincs, len, 0);
		if (irecv > 0)
		{
			if ( irecv < 4 || logincs[0] !='N' ) return false; // 非法开头
			logincs[irecv] = '\0';
			string loginStr = logincs;
			int subStartIndex = 2;
			while (subStartIndex < irecv)
			{
				if (loginStr[subStartIndex]==',') break;
				else subStartIndex++;
			}
			if (subStartIndex == irecv) return false; // 密码为空
			char* name = new char[ (size_t)subStartIndex - 1 ];
			char* pass = new char[ ( (size_t)irecv - subStartIndex ) ];
			loginStr.copy( name, (size_t)subStartIndex - 2, 2 );
			name[ (size_t)subStartIndex - 2 ] = '\0';
			loginStr.copy(pass, ( (size_t)irecv - subStartIndex ), (size_t)subStartIndex +1);
			pass[ ( (size_t)irecv - subStartIndex ) - 1 ] = '\0';
			for (vector<Client*>::iterator it = clients.begin(); it != clients.end(); it++)
			{
				Client* p_C = (Client *)(*it);
				if ((*p_C) == name)
				{
					cout << "已有登录" << endl;
					delete[] name;
					delete[] pass;
					return false;
				}
			}

			for (vector<User>::iterator it = users.begin(); it != users.end(); it++)
			{
				if (strcmp(it->getname(), name) == 0)
				{
					if (it->login_verification(pass)) 
					{
						delete[] name;
						delete[] pass;
						return true;
					}
					cout << "密码错误" << endl;
					delete[] name;
					delete[] pass;
					return false;
				}
				uIndex ++;
			}
			cout << "用户不存在" << endl;
			delete[] name;
			delete[] pass;
			return false;
		}
		recvMaxTime--;
	}
	cout << "超时" << endl;
	return false;
}

void ChatServer::stopServer()
{
	// 通知服务器下线
	CleanDeadClient();
	clients.clear();
	router.StopWork();
	WSACleanup();
	listenSocket = INVALID_SOCKET;
}

bool ChatServer::init_server()
{
	cout << "init server..." << endl;
	// 初始化
	WSADATA  wsaData;
	int iResult;
	unsigned long ul = 1;
	struct addrinfo* result = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return false;
	}
	// 解析ip地址和端口
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return false;
	}
	// 创建待连接套接字
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return false;
	}
	iResult = ioctlsocket(listenSocket, FIONBIO, (unsigned long*)&ul);
	if (iResult == SOCKET_ERROR)
	{
		printf("Set block or not failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	// 绑定套接字监听
	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	// 不再需要result，释放内存
	freeaddrinfo(result);

	return true;
}

unsigned WINAPI SubMessageRouter::onMessageExecuteThread(void* param)
{
	SubMessageRouter* pRouter = (SubMessageRouter*)param;
	if (pRouter != NULL)
	{
		cout << "SubMessageRouter start..." << endl;
		while (pRouter->IsExec)
		{
			string oneMess;
			m_queue.DeQueue(oneMess);
			if (!oneMess.empty())
			{
				/*
				处理队列消息
				Login_Mess:N, ---->>send 'N,userName1,UserName2,UserName3...' to All;
				Chat_Mess:Y,d_userName,s_userName,content ---->>send 'Y,s_userName,content' to d_userName;
				UnLogin_Mess:W,s_userName ---->>send 'W,s_userName' to All.
				*/
				cout<<"DeQueue a mess: " << oneMess << endl;
				if ( oneMess[0] == 'N' )
				{
					for (vector<Client*>::iterator iter = clients.begin(); iter != clients.end(); iter++)
					{
						oneMess.append((*iter)->getUserName());
						oneMess.append(",");
					}
					cout<<"send to all: " << oneMess << endl;
					pRouter->SendAll(oneMess);
				}
				else if ( oneMess[0] == 'Y' )
				{
					int dNameEndIndex = 2;
					while (dNameEndIndex < oneMess.size() )
					{
						if ( oneMess[dNameEndIndex] == ',' ) break;
						else dNameEndIndex++;
					}
					/*int contentStartIndex = dNameEndIndex + 1;
					while (contentStartIndex < oneMess.size())
					{
						if (oneMess[contentStartIndex] == ',') break;
						else contentStartIndex++;
					}*/
					char* name = new char[(size_t)dNameEndIndex - 1];
					name[(size_t)dNameEndIndex - 2] = '\0';
					oneMess.copy(name, (size_t)dNameEndIndex - 2, 2);
					//oneMess.erase(0, (size_t)contentStartIndex +1);
					if (strcmp(name,"ALL") == 0)
					{
						pRouter->SendAll(oneMess);
					}
					else 
					{
						pRouter->SendOne(name, oneMess);
					}
				}
				else if ( oneMess[0] == 'W' )
				{
					pRouter->SendAll(oneMess);
				}
				else
				{
					cout << "error mess" << endl;
				}
			}
			else
			{
				//cout << "no mess" << endl;
				Sleep(3000);
			}
		}
	}
	return 0;
}

void SubMessageRouter::SendAll(string& s)
{
	for (vector<Client*>::iterator iter = clients.begin(); iter != clients.end(); iter++)
	{
		while (!(*iter)->onSend(s));
	}
}

void SubMessageRouter::SendOne(char* d_username, string& s)
{
	for (vector<Client*>::iterator iter = clients.begin(); iter != clients.end(); iter++)
	{
		if ((*(*iter)) == d_username)
		{
			while ( !(*iter)->onSend(s) );
			return;
		}
	}
	cout << "no one" << endl;
}

bool SubMessageRouter::DoWork()
{
	if (execHandle || IsExec)
	{
		return false;
	}
	IsExec = true;
	execHandle = (HANDLE)_beginthreadex(NULL, 0, &onMessageExecuteThread, this, 0, 0);
	return execHandle != NULL;
}

void SubMessageRouter::StopWork()
{
	this->IsExec = false;
}

SubMessageRouter::SubMessageRouter()
{
	IsExec = false;
	execHandle = NULL;
}
