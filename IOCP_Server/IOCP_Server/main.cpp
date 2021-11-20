#define _WINSOCKAPI_
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
	// 서버 설정
	iocpMain->GetInstance()->InitIOCP(maxThreads, maxSockets, port);
	iocpMain->GetInstance()->InitThreadPool(maxThreads);
	while (TRUE) {};
	iocpMain->~IocpMain();

	CloseHandle(iocpMain->hAcceptIOCP); 
	CloseHandle(iocpMain->hWorkerIOCP);
}