#include "Thread.h"

Thread::Thread() : m_pkHandle(0), m_Id(0)
{

}

Thread::~Thread() 
{
	Stop();
}

unsigned WINAPI Thread::ThreadProc(void* pvDerivedThread) 
{
	Thread* pThread = (Thread*)pvDerivedThread;
	if (pThread) {
		pThread->DoWork();
	}
	return 0;
}

bool Thread::Start()
{
	if (m_pkHandle || this->IsRunning()) {
		return false;
	}
	m_pkHandle = (HANDLE)_beginthreadex(NULL, 0, &ThreadProc, this, 0, &m_Id);
	return m_pkHandle != NULL;
}

void Thread::Stop()
{
	if (!m_pkHandle) {
		return;
	}
	CloseHandle((HANDLE)m_pkHandle);
	m_pkHandle = NULL;
}

bool Thread::IsRunning() const
{
	if (m_pkHandle) 
	{
		DWORD exitCode = 0;
		if (GetExitCodeThread((HANDLE)m_pkHandle, &exitCode)) 
		{
			if (STILL_ACTIVE == exitCode) 
			{
				return true;
			}
		}
	}
	return false;
}

bool Thread::WaitForWork(DWORD  dwMilliseconds)
{
	if ( !IsRunning())
	{
		return false;
	}
	else
	{
		DWORD waitResult = WaitForSingleObject((HANDLE)m_pkHandle, dwMilliseconds);
		if (waitResult == WAIT_OBJECT_0) return true;
		else return false;
	}
}

bool Thread::WaitForWork()
{
	return WaitForWork(INFINITE);
}

unsigned int Thread::GetId() 
{
	return m_Id;
}
