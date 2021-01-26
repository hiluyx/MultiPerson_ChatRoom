#pragma once
#include "User.h"
#include "Thread.h"
#include <Windows.h>

class Client
{
public:
    Client(SOCKET s, User *u);
    ~Client();

    bool IsConnected();//判断连接是否中断。
    bool DisConnect();//中断与服务器的连接。
    bool IsBusy();
    void SetBusy(bool b);
    bool startRunning();//开始运行发送和接收线程。

    static DWORD WINAPI sendThread(void* param);//发送线程入口函数。
    static DWORD WINAPI recvThread(void* param);//接收线程入口函数。

    bool operator==(char* name);
    bool operator!=(char* name);

    char* getUserName();
    bool onSend(string& mess);
private:
	User *user;

    HANDLE m_hSendThread;//发送线程句柄。
    HANDLE m_hRecvThread;//接受线程句柄。
    HANDLE m_hEvent;//发送线程和接收线程同步事件对象。接收客户端请求后通知发送线程发送当前时间。

    SOCKET m_socket;//与客户端连接套接字。
    bool m_IsConnected;
    bool m_IsBusy;
    char* m_pRecvData;//接收缓冲区。
    char* m_pSendData;//发送缓冲区。
    bool m_IsSendData;//由于只有接收到客户端请求后才需要发送，该变量控制是否发送数据。
};