#include "IOCP_main.h"
#include "ServerData.h"
#include "Define.h"
#include "SocketInfo.h"
#include "ChannelData.h"
#include <iostream>

using namespace std;

int main()
{
	IocpMain* iocpMain=nullptr;

	int maxThreads = 4, port = 1500, maxSockets = 4;
	int zero = 0;
	iocpMain->GetInstance()->InitIOCP(maxThreads, maxSockets, port);
	iocpMain->GetInstance()->InitThreadPool(maxThreads);
	while (TRUE) {};
	iocpMain->~IocpMain();

	CloseHandle(iocpMain->hAcceptIOCP); // shut down winsock WSACleanup(); return 0;
	CloseHandle(iocpMain->hWorkerIOCP);
}