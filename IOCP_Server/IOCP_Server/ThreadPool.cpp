#include "ThreadPool.h"
#include "SocketInfo.h"

ThreadPool::ThreadPool()
{
	threadNum = 0;
	isStop = false;

}

ThreadPool::~ThreadPool()
{
	isStop = true;
	cvUserWorkQueue.notify_all();

	for (auto& t : vProcessWork)
	{
		t.join();
	}
}

bool ThreadPool::InitTreadPool(int threadNum_)
{
	threadNum = threadNum_;
	isStop = false;

	vProcessWork.reserve(threadNum);
	unsigned int dummy;
	for (int i = 0; i < threadNum; i++)
	{
		vProcessWork.emplace_back([this]() { this->ProcessWorkThrad(); });
	}
	return true;
}

unsigned int __stdcall ThreadPool::ProcessWorkThrad()
{
	while (1)
	{
		unique_lock<mutex> lock(mutexUserWorkQueue);
		cvUserWorkQueue.wait(lock, [this]() {return !this->qUserWorkQueue.empty() || isStop; });
		if (isStop && qUserWorkQueue.empty())
		{
			return 0;
		}

		function<void()> userWorkFunc = qUserWorkQueue.front();
		qUserWorkQueue.pop();
		lock.unlock();

		userWorkFunc();

	}
}

void ThreadPool::EnqueueWork(function<void()> work)
{
	if (isStop)
		return;
	{
		lock_guard<mutex> lock(mutexUserWorkQueue);
		qUserWorkQueue.push(work);
	}
	cvUserWorkQueue.notify_one();
}