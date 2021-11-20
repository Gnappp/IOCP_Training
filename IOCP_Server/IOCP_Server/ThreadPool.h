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

// ������ ��û���װ� �����ͺ��̽� ������ ����ϴ� 
// �����带 �����ϴ� Ŭ����
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

	// ����������Ŷ
	bool isStop;
	vector<thread> vProcessWork;
	queue<UserPacketData*> userPacketQueue; //������Ŷť
	mutex mutexUserWorkQueue;
	condition_variable cvUserWorkQueue;
	// ����������Ŷ End
	
	// DB ��Ŷ
	bool isDbStop;
	char provider[1024];
	vector<thread> vDbProcessWork;
	queue<DbPacketData*> dbPacketQueue; // �����ͺ��̽� ť
	mutex mutexDbWorkQueue;
	condition_variable cvDbWorkQueue;
	// DB ��Ŷ End

	unsigned int __stdcall ProcessWorkThrad();
	unsigned int __stdcall ProcessDbWorkThrad();
};