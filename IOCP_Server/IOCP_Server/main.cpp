#include "IOCP_main.h"
#include "ServerData.h"
#include "Define.h"
#include "SocketInfo.h"
#include "ChannelData.h"

IocpMain* iocpMain;

int main()
{
	iocpMain = new IocpMain;

	int maxThreads = 4, port = 1500, maxSockets = 4;
	int zero = 0;
	
	iocpMain->InitIOCP(maxThreads, maxSockets, port);
	iocpMain->InitThreadPool(maxThreads);
	while (TRUE) {};
	iocpMain->~IocpMain();

	CloseHandle(iocpMain->hAcceptIOCP); // shut down winsock WSACleanup(); return 0;
	CloseHandle(iocpMain->hWorkerIOCP);
}