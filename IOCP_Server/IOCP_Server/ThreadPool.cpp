#include "ThreadPool.h"
#include "SocketInfo.h"
#include "ServerData.h"
#include <process.h>

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
	string tmp_provider = ""; // ADODB 정보 입력
	SetProvider(tmp_provider.c_str(), tmp_provider.size());
	for (int i = 0; i < threadNum; i++)
	{
		vProcessWork.emplace_back([this]() { this->ProcessWorkThrad(); });
		vDbProcessWork.emplace_back([this]() {this->ProcessDbWorkThrad(); });
	}
	return true;
}

unsigned int __stdcall ThreadPool::ProcessWorkThrad()
{
	while (1)
	{
		unique_lock<mutex> lock(mutexUserWorkQueue);
		cvUserWorkQueue.wait(lock, [this]() {return !this->userPacketQueue.empty() || isStop; });
		if (isStop && userPacketQueue.empty())
		{
			return 0;
		}

		UserPacketData* userPacketData = userPacketQueue.front();
		userPacketQueue.pop();
		short commandType = 0;
		CopyMemory(&commandType, userPacketData->userPacket, sizeof(short));
		cout << userPacketData->userSocketData->userSockIndex << "  Command Type :" << commandType << endl;
		switch (commandType)
		{
		case REQUEST_LOGIN:
		{
			RequestLogin(userPacketData);
			break;
		}
		case REQUEST_JOIN_CHANNEL:
		{
			RequestJoinChannel(userPacketData);
			break;
		}
		case REQUEST_EXIT_CHANNEL:
		{
			RequestExitChannel(userPacketData);
			break;
		}
		case REQUEST_CREATE_ROOM:
		{
			RequestCreateRoom(userPacketData);
			break;
		}
		case REQUEST_JOIN_ROOM:
		{
			RequestJoinRoom(userPacketData);
			break;
		}
		case REQUEST_CHAT_ROOM:
		{
			RequestChatRoom(userPacketData);
			break;
		}
		case REQUEST_EXIT_ROOM:
		{
			RequestExitRoom(userPacketData);
			break;
		}
		default:
			
			break;
		}

		lock.unlock();
	}
}

void ThreadPool::EnqueueWork(UserPacketData* packet)
{
	if (isStop)
		return;
	{
		lock_guard<mutex> lock(mutexUserWorkQueue);
		userPacketQueue.push(packet);
	}
	cvUserWorkQueue.notify_one();
}

void ThreadPool::EnqueueDbWork(DbPacketData* qry)
{
	if (isDbStop)
		return;
	{
		lock_guard<mutex> lock(mutexDbWorkQueue);
		dbPacketQueue.push(qry);
	}
	cvDbWorkQueue.notify_one();
}

void ThreadPool::SetProvider(const char* provider, int size)
{
	CopyMemory(this->provider, provider, size);
}

unsigned int __stdcall ThreadPool::ProcessDbWorkThrad()
{
	CAdodb db;
	db.Connect(provider);
	while (1)
	{
		unique_lock<mutex> lock(mutexDbWorkQueue);
		cvDbWorkQueue.wait(lock, [this]() {return !this->dbPacketQueue.empty() || isStop; });
		if (isDbStop && dbPacketQueue.empty())
		{
			return 0;
		}
		DbPacketData* dbPacketData = dbPacketQueue.front();
		dbPacketQueue.pop();
		db.ExcuteQry(dbPacketData->qry);
		switch (dbPacketData->qryType)
		{
		}
	}
}