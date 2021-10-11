#include "IOCP_main.h"
#include "SocketInfo.h"
#include "Define.h"
#include "UserData.h"
#include "RoomData.h"
#include "ChannelData.h"
#include "ServerData.h"
#include "ThreadPool.h"

IocpMain* IocpMain::inst = nullptr;

IocpMain* IocpMain::GetInstance()
{
	if (inst == nullptr)
		inst = new IocpMain();

	return inst;
}

IocpMain::IocpMain()
{
	//ch.clear();
	//rm.clear();
	//mUserDatas.clear();
	//vSocketData.clear(); //소켓 데이터들
	sockListen = INVALID_SOCKET;
	threadPool=nullptr;
	hAcceptIOCP = nullptr;
	hWorkerIOCP = nullptr;
}

IocpMain::~IocpMain()
{
	for (int i = 0; i < MAX_CHANNEL; i++)
		delete(&ch[i]);
	for (vector<RoomData* >::iterator iter = rm.begin(); iter != rm.end(); iter++)
		delete(&iter);
	for (int i = 0; i < mUserDatas.size(); i++)
		delete(mUserDatas[i]);
	for (int i = 0; i < vSocketData.size(); i++)
		delete(vSocketData[i]);
	delete(threadPool);
	delete(hAcceptIOCP);
	delete(hWorkerIOCP);
	for (iter_vSocketData iter = vSocketData.begin(); iter != vSocketData.end(); iter++)
	{
		closesocket((*iter)->sock);
		delete (&iter);
	}
}

bool IocpMain::InitIOCP(int maxthreads, int maxsockets, int port)
{
	HANDLE h;
	unsigned int dummy;

	hAcceptIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	hWorkerIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (hAcceptIOCP == NULL || hWorkerIOCP == NULL)
		return 0;

	//Create Thread
	for (int i = 0; i < maxthreads; i++)
	{
		h = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, &dummy);
		if (h == NULL) return 0;
		CloseHandle(h);
	}

	for (int i = 0; i < maxthreads; i++)
	{
		h = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, NULL, 0, &dummy);
		if (h == NULL) return 0;
		CloseHandle(h);
	}

	WSADATA wd = { 0, };
	WSAStartup(MAKEWORD(2, 2), &wd);

	//Create sockListen
	SOCKADDR_IN addr;

	sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons((short)port);
	int result;
	result = ::bind(sockListen, (sockaddr*)&addr, sizeof(addr));
	if (result == SOCKET_ERROR)
		return 0;
	if (listen(sockListen, 200) == SOCKET_ERROR)
		return 0;

	//Create accept Overlap structur
	vSocketData.clear();
	for (int i = 0; i < maxsockets; i++)
	{
		SocketData* sockData = new SocketData(sockListen, i);
		//InitOVERLAP(*pOverlap);
		vSocketData.push_back(sockData);
	}

	CreateIoCompletionPort((HANDLE)sockListen, hAcceptIOCP, 0, 0);

	for (int i = 0; i < MAX_CHANNEL; i++)
	{
		ch.push_back(new ChannelData(i));
	}
}

bool IocpMain::InitThreadPool(int maxthreads)
{
	threadPool = new ThreadPool();
	if (!threadPool->InitTreadPool(maxthreads))
		return false;
	return true;
}