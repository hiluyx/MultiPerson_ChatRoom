#pragma once
#include <windows.h>
#include <process.h>
#include <string>
#include <iostream>
using namespace std;

class Thread
{
public:
	Thread();
	virtual ~Thread();
	bool Start();
	bool IsRunning() const;
	unsigned int GetId();
	bool WaitForWork();
	bool WaitForWork(DWORD  dwMilliseconds);
protected:
	void Stop();
	virtual void DoWork() {}
private:
	static unsigned WINAPI ThreadProc(void* pvDerivedThread);
	void*             m_pkHandle;
	string            m_kName;
	unsigned int m_Id;
};
