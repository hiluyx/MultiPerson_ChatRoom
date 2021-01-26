#include "Client.h"
#include "LoopQueue.h"

extern LoopQueue m_queue;
Client::Client(SOCKET s, User* u) :m_socket(s), user(u)
{
    cout<<"new Client:" << u->getname() << endl;
    m_hRecvThread = NULL;
    m_hSendThread = NULL;
    m_IsConnected = true;
    m_IsSendData = true;
    m_hEvent = CreateEvent(NULL, true, false, NULL);
    m_pRecvData = new char[256];
    m_pSendData = new char[256];
    memset(m_pSendData, 0, 256);
    memset(m_pRecvData, 0, 256);
}

bool Client::IsConnected()
{
    return m_IsConnected;
}

Client::~Client()
{
    delete[] m_pRecvData;
    delete[] m_pSendData;
}

char* Client::getUserName()
{
    return this->user->getname();
}

bool Client::onSend(string& mess)
{
    if (this->m_IsBusy)
    {
        return false;
    }
    mess.copy(this->m_pSendData, mess.size(), 0);
    SetEvent(this->m_hEvent); // 通知发送线程
    return true;
}

DWORD WINAPI Client::sendThread(void* param)  //发送线程入口函数。  
{
    Client* pClient = static_cast<Client*>(param);  
    while (pClient->m_IsConnected)
    {
        cout << "wait for send a mess" << endl;
        WaitForSingleObject(pClient->m_hEvent, INFINITE);
        pClient->SetBusy(true);
        if (!pClient->m_IsConnected)
        {
            break;
        }
        cout << "send now" << endl;
        int ret = send(pClient->m_socket, pClient->m_pSendData, 256, 0);
        if (ret != SOCKET_ERROR)
        {
            pClient->m_IsSendData = false;
            cout << "send successfully" << endl;
        }
        else
        {
            cout << "send fail" << endl;
            int r = WSAGetLastError();
            if (r == WSAEWOULDBLOCK)
            {
                continue;
            }
            else
            {
                cout << "send return" << endl;
                return 0;
            }
        }
        pClient->SetBusy(false);
        ResetEvent(pClient->m_hEvent);
    }
    cout << "send thread end." << endl;
    return 0;
}

DWORD WINAPI Client::recvThread(void* param)  //接收数据线程入口函数。
{
    Client* pClient = static_cast<Client*>(param);
    while (pClient->m_IsConnected)
    {
        memset(pClient->m_pRecvData, 0, 256);
        int ret = recv(pClient->m_socket, pClient->m_pRecvData, 256, 0);
        if (ret != SOCKET_ERROR)
        {
            pClient->m_pRecvData[ret] = '\0';
            cout << "recv from:" << pClient->m_pRecvData << endl;
            string enqueueString = pClient->m_pRecvData;
            // 聊天消息进队
            if (!m_queue.EnQueue(enqueueString))
            {
                cout << "Server Busy" << endl;
            }
        }
        else
        {
            int r = WSAGetLastError();
            if (r == WSAEWOULDBLOCK)
            {
                Sleep(20);
                continue;
            }
            else if (r == WSAENETDOWN)
            {
                pClient->DisConnect();
                break;
            }
            else
            {
                pClient->DisConnect();
                cout << "connection broken." << endl;
                // 通知下线消息进队
                string s;
                s.append("W,");
                s.append(pClient->user->getname());
                m_queue.EnQueue(s);
                break;
            }
        }
    }
    return 0;
}

bool Client::operator==(char* name)
{
    return strcmp(this->user->getname(), name) == 0;
}

bool Client::operator!=(char* name)
{
    return strcmp(this->user->getname(), name) != 0;
}


bool Client::startRunning()  //开始为连接创建发送和接收线程。  
{
    m_hRecvThread = CreateThread(NULL, 0, recvThread, (void*)this, 0, NULL);
    if (m_hRecvThread == NULL)
    {
        return false;
    }
    m_hSendThread = CreateThread(NULL, 0, sendThread, (void*)this, 0, NULL);
    if (m_hSendThread == NULL)
    {
        return false;
    }
    return true;
}

bool Client::DisConnect()
{
    m_IsConnected = false;  //接收和发送线程退出。资源释放交由资源释放线程。  
    return true;
}

bool Client::IsBusy()
{
    return m_IsBusy;
}

void Client::SetBusy(bool b)
{
    this->m_IsBusy = b;
}
