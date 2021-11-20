#pragma once
#include "AllLib.h"
#include "Define.h"
#include "ADODB.h"

#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

using namespace std;

class SocketData;

// 유저의 요청사항과 데이터베이스 업무를 담당하는 
// 스레드를 관리하는 클래스
class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();
	bool InitTreadPool(int threadNum_);

	void EnqueueWork(UserPacketData* packet);

	void EnqueueDbWork(DbPacketData* qry);
	void SetProvider(const char* provider, int size);



private:
	int threadNum;

	// 유저게임패킷
	bool isStop;
	vector<thread> vProcessWork;
	queue<UserPacketData*> userPacketQueue; //유저패킷큐
	mutex mutexUserWorkQueue;
	condition_variable cvUserWorkQueue;
	// 유저게임패킷 End
	
	// DB 패킷
	bool isDbStop;
	char provider[1024];
	vector<thread> vDbProcessWork;
	queue<DbPacketData*> dbPacketQueue; // 데이터베이스 큐
	mutex mutexDbWorkQueue;
	condition_variable cvDbWorkQueue;
	// DB 패킷 End

	unsigned int __stdcall ProcessWorkThrad();
	unsigned int __stdcall ProcessDbWorkThrad();
};