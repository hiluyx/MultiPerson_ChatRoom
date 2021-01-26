#pragma once
#include "User.h"
#include "Thread.h"
#include <Windows.h>

class Client
{
public:
    Client(SOCKET s, User *u);
    ~Client();

    bool IsConnected();//�ж������Ƿ��жϡ�
    bool DisConnect();//�ж�������������ӡ�
    bool IsBusy();
    void SetBusy(bool b);
    bool startRunning();//��ʼ���з��ͺͽ����̡߳�

    static DWORD WINAPI sendThread(void* param);//�����߳���ں�����
    static DWORD WINAPI recvThread(void* param);//�����߳���ں�����

    bool operator==(char* name);
    bool operator!=(char* name);

    char* getUserName();
    bool onSend(string& mess);
private:
	User *user;

    HANDLE m_hSendThread;//�����߳̾����
    HANDLE m_hRecvThread;//�����߳̾����
    HANDLE m_hEvent;//�����̺߳ͽ����߳�ͬ���¼����󡣽��տͻ��������֪ͨ�����̷߳��͵�ǰʱ�䡣

    SOCKET m_socket;//��ͻ��������׽��֡�
    bool m_IsConnected;
    bool m_IsBusy;
    char* m_pRecvData;//���ջ�������
    char* m_pSendData;//���ͻ�������
    bool m_IsSendData;//����ֻ�н��յ��ͻ�����������Ҫ���ͣ��ñ��������Ƿ������ݡ�
};