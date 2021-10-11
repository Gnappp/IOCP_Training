#pragma once
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

using namespace std;

class SocketData;

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	void EnqueueWork(function<void()> work);
	bool InitTreadPool(int threadNum_);
private:
	int threadNum;
	vector<thread> vProcessWork;
	queue<function<void()>> qUserWorkQueue;
	mutex mutexUserWorkQueue;
	condition_variable cvUserWorkQueue;
	unsigned int __stdcall ProcessWorkThrad();
	bool isStop;
};