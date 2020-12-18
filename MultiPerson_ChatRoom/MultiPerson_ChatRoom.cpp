// server_2.cpp : 定义控制台应用程序的入口点。、
//1、设计要求
//使用Socket实现网上聊天功能。
//用户可以通过客户端连接到服务器端并进行网上聊天。聊天时可以启动多个客户端。
//服务器端启动后，接收客户端发来的用户名和密码验证信息。
//验证通过则以当前的聊天客户列表信息进行响应；
//此后接收客户端发来的聊天信息，转发给客户端指定的聊天客户（即私聊）或所有其他客户端；
//在客户断开连接后公告其退出聊天系统的信息。
//客户端启动后在GUI界面接收用户输入的服务器端信息、账号和密码等验证客户的身份。
//验证通过则显示当前系统在线客户列表。
//客户可以与指定对象进行私聊，也可以向系统中所有在线客户发送信息。

//实现本程序需要了解网络基础知识，掌握C / S结构的工作特点，掌握数据结构、高级语言及网络编程知识，可以选择Visual C++、C或Java等语言实现。

//2、课程设计报告内容
//(1) 给出系统的结构；
//(2) 给出程序的流程图；
//(3) 分别给出服务器端和客户端的程序源码；
//(4) 给出程序的部分运行测试结果。
//
 
#include <stdio.h>
 
#include <iostream>   
#include <cassert>   
#include <list>   
#include <WINSOCK2.h>
#pragma comment(lib,"WS2_32.LIB")  
#define ASSERT assert 
using namespace std;
      
static const int c_iPort = 10001;   
bool GraceClose(SOCKET *ps); 
 
int main()
{
	int iRet = SOCKET_ERROR;
 
	// 初始化Winsocket   
	WSADATA data;   
	ZeroMemory(&data, sizeof(WSADATA));   
	iRet = WSAStartup(MAKEWORD(2, 0), &data);   
	ASSERT(SOCKET_ERROR != iRet);  
 
	// 建立服务端程序的监听套接字
	SOCKET skListen = INVALID_SOCKET;   
	skListen = socket(AF_INET, SOCK_STREAM, 0);   
	ASSERT(INVALID_SOCKET != skListen);  
 
	// 初始化监听套接字地址信息   
	sockaddr_in adrServ;  
	ZeroMemory(&adrServ, sizeof(sockaddr_in));   
	adrServ.sin_family      = AF_INET;           
	adrServ.sin_port        = htons(c_iPort);     
	adrServ.sin_addr.s_addr = INADDR_ANY; 
 
	// 绑定监听套接字到本地   
	iRet = bind(skListen, (sockaddr*)&adrServ, sizeof(sockaddr_in));   
	ASSERT(SOCKET_ERROR != iRet);   
 
	// 使用监听套接字进行监听   
	iRet = listen(skListen, FD_SETSIZE);  
	ASSERT(SOCKET_ERROR != iRet);
 
	// 将套接口从阻塞状态设置到费阻塞状态   
	unsigned long ulMode = 1;   
	iRet = ioctlsocket(skListen, FIONBIO, &ulMode);   
	ASSERT(SOCKET_ERROR != iRet); 
 
	fd_set fsListen;   //fd_set为套字集合
	FD_ZERO(&fsListen); //FD_ZERO(*set)将集合set清空  
	fd_set fsRead;   
	FD_ZERO(&fsRead); 
 
	timeval tv;   
	tv.tv_sec  = 1;   
	tv.tv_usec = 0; 
 
	list<SOCKET> sl;   
	int i = 2;
	for(;;)   
	{  
		// 接收来自客户端的连接, 并将新建的套接字加入套接字列表中   
		FD_SET(skListen, &fsListen);//FD_SET(s, *set)将套接字s加入到集合set中
		iRet = select(1, &fsListen, NULL, NULL, &tv); 
		if(iRet > 0)   
		{   
			sockaddr_in adrClit;   
			int iLen = sizeof(sockaddr_in);   
			ZeroMemory(&adrClit, iLen);   
			SOCKET skAccept = accept(skListen, (sockaddr*)&adrClit,&iLen);   
			ASSERT(INVALID_SOCKET != skAccept);   
			sl.push_back(skAccept); 
			cout << "New connection " << skAccept << ", c_ip: " 
				<< inet_ntoa(adrClit.sin_addr) << ", c_port: " << ntohs(adrClit.sin_port) << endl; 
		} 
 
		// 将套接字列表中的套接字加入到可读套接字集合中,以便在可以检测集合中的套接字是否有数据可读   
		FD_ZERO(&fsRead);   
		for(list<SOCKET>::iterator iter = sl.begin(); iter != sl.end(); ++iter)   
		{   
			FD_SET(*iter, &fsRead);   
		} 
 
		// 检测集合中的套接字是否有数据可读   
		iRet = select(sl.size(), &fsRead, NULL, NULL, &tv);   
		if(iRet > 0)   
		{   
			for(list<SOCKET>::iterator iter = sl.begin(); iter != sl.end(); ++iter)   
			{   
				// 如果有数据可读, 则遍历套接字列表中的所有套接字,检测出有数据可读的套接字   
				iRet = FD_ISSET(*iter, &fsRead);// FD_ISSET(s, *set)判断套接字s是否在集合中有信号
				if(iRet > 0)   
				{   
					// 读取套接字上的数据   
					const int c_iBufLen = 512;   
					char recvBuf[c_iBufLen + 1] = {'\0'};   
					int iRecv = SOCKET_ERROR;   
					iRecv = recv(*iter, recvBuf, c_iBufLen, 0);
 
					if (iRecv <= 0 )// 读取出现错误或者对方关闭连接   
					{   
					   iRecv == 0 ? cout << "Connection shutdown at socket " << *iter << endl ://优雅关闭
							cout << "Connection recv error at socket " << *iter << endl;// 粗暴关闭
						iRet = GraceClose(&(*iter));   
						ASSERT(iRet);   
					}   
					else  
					{   
						recvBuf[iRecv] = '\0';   
						cout << "Server recved message from socket " << *iter << ": " << recvBuf << endl;   
						
						// 创建可写集合   
						FD_SET fsWrite;   
						FD_ZERO(&fsWrite);   
						FD_SET(*iter, &fsWrite);   
						// 如果可可写套接字, 则向客户端发送数据   
						iRet = select(1, NULL, &fsWrite, NULL, &tv);   
						if (iRet <= 0)   
						{   
							int iWrite = SOCKET_ERROR;   
							iWrite = send(*iter, recvBuf, iRecv, 0);   
							if (SOCKET_ERROR == iWrite)   
							{   
								cout << "Send message error at socket " << *iter << endl;   
								iRet = GraceClose(&(*iter));   
								ASSERT(iRet);   
							}   
						}   
					}   
				}   
			}  //for 
			sl.remove(INVALID_SOCKET); // 删除无效的套接字, 套接字在关闭后被设置为无效   
		}  //if 
	}  //for (;;) 
 
	// 将套接字设置回阻塞状态   
	ulMode = 0;   
	iRet = ioctlsocket(skListen, FIONBIO, &ulMode);   
	ASSERT(SOCKET_ERROR != iRet);  
 
	// 关闭监听套接字   
	iRet = GraceClose(&skListen);   
	ASSERT(iRet);  
 
	// 清理Winsocket资源   
	iRet = WSACleanup();   
	ASSERT(SOCKET_ERROR != iRet); 
 
	system("pause");   
	return 0;   
}   
bool GraceClose(SOCKET *ps)   
{   
	const int c_iBufLen = 512;   
	char szBuf[c_iBufLen + 1] = {'\0'};  
 
	// 关闭该套接字的连接   
	int iRet = shutdown(*ps, SD_SEND);   
	while(recv(*ps, szBuf, c_iBufLen, 0) > 0);   
	if (SOCKET_ERROR == iRet)   
	{   
		return false;   
	}   
 
	// 清理该套接字的资源   
	iRet = closesocket(*ps);   
	if (SOCKET_ERROR == iRet)   
	{   
		return false;   
	}   
 
	*ps = INVALID_SOCKET;   
	return true;   
}  