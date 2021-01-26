#include "LoopQueue.h"
#include<Windows.h>

LoopQueue::LoopQueue(int size):qsize(size)
{

} 
bool LoopQueue::EnQueue(string s)
{
	int nowEnQueueTime = 0;
	while (nowEnQueueTime < MaxEnQueueTime)
	{
		if (mq.size() > qsize)
		{
			Sleep(1000);
			nowEnQueueTime++;
			continue;
		}
		else 
		{
			lock.lock();
			mq.push(s);
			lock.unlock();
			return true;
		}
	}
	return false;
}

bool LoopQueue::empty()
{
	lock.lock();
	bool isEmpty = mq.empty();
	lock.unlock();
	return isEmpty;
}

void LoopQueue::DeQueue(string& strTmp)
{
	if (mq.empty())
	{
		return ;
	}
	lock.lock();
	string popStr = mq.front();
	mq.pop();
	lock.unlock();
	strTmp = popStr;
}
