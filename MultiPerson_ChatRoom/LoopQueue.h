#pragma once
#include <iostream>
#include<queue>
#include<string>
#include <mutex>

using namespace std;
static int MaxEnQueueTime = 10;
class LoopQueue
{
private:
	queue<string> mq;
	int qsize;
	mutex lock;
public:
	LoopQueue(int size = 10);
	bool EnQueue(string s);
	bool empty();
	void DeQueue(string& strTmp);
};

