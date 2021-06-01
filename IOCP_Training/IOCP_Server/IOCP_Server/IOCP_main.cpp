#pragma once
#include <iostream> 
#include <WinSock2.h> 
#include "IOCP_main.h"

// 초기화할때 AcceptEx를 이용하여 리슨될때 생성한 소켓으로 갈 수 있게 해준다.
// AcceptEx(리슨소켓, 만들어진넣을소켓, 수신된 데이터를 받을 버퍼[char []],	받을 데이터크기[int], 소켓 ip크기[int], 소켓 ip크기, 받은데이터 크기[DWORD], overlap 구조체
// GetAcceptExSockaddrs(overlap의 버퍼크기,	받을 데이터크기, 소켓 ip크기, 소켓 ip크기, 로컬ip받을 sockaddr_in, 로컬 ip 크기[int], 리모트 ip받을 sockaddr_in, 리모트 ip 크기[int])

SocketData::SocketData(SOCKET& listenSock,int iSock_num)
{
	memset(this, NULL, sizeof(*this));
	int zero = 0;
	isConnected = false;
	sock_num = iSock_num;
	ZeroMemory(bufFront, BUF_SIZE);
	ZeroMemory(&recvBuf, sizeof(WSABUF));
	sendData = { 0, };

	bufEnd = &bufFront[0];
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		cout << "Fail create sock" << endl;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero));
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero));

	sendOverLap.state = SEND;

	recvOverLap.state = RECV;
	recvOverLap.sockData = this;
	AcceptEx(listenSock, sock, bufEnd, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &byteSize, (OVERLAPPED*)&recvOverLap);
}

unsigned int __stdcall AcceptThread(void* arg)
{
	DWORD nByteSize, key;
	BOOL result;
	string str;
	stOverlap* overlap;
	int localLen, remoteLen;
	sockaddr_in* local = 0, * remote = 0;

	while (1)
	{
		result = GetQueuedCompletionStatus(hAcceptIOCP, &nByteSize, (PULONG_PTR)&key, (OVERLAPPED**)&overlap, INFINITE);
		cout << "Accept Connect!" << endl;

		if (!result || overlap == NULL)
		{
			if (overlap != NULL)
			{
				cout << "Accept socket close" << endl;
				CloseSocket(*(overlap->sockData), true, sockListen == INVALID_SOCKET ? false : true);
			}
		}
		else
		{
			if (!overlap->sockData->isConnected)
			{
				GetAcceptExSockaddrs(&overlap->sockData->bufFront[0], 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr**)&local, &localLen, (sockaddr**)&remote, &remoteLen);
				memcpy(&overlap->sockData->mLocal, local, sizeof(sockaddr_in));
				memcpy(&overlap->sockData->mRemote, remote, sizeof(sockaddr_in));
				setsockopt(overlap->sockData->sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&sockListen, sizeof(sockListen));
				overlap->sockData->isConnected = true;

				CreateIoCompletionPort((HANDLE)overlap->sockData->sock, hWorkerIOCP, (DWORD)overlap->sockData, 0);
				PostRead(*(overlap->sockData));
			}
		}
	}
}

unsigned int __stdcall WorkerThread(void* arg)
{
	DWORD nByteSize, key;
	BOOL result;
	stOverlap* overlap;

	while (1)
	{
		result = GetQueuedCompletionStatus(hWorkerIOCP, &nByteSize, (PULONG_PTR)&key, (OVERLAPPED**)&overlap, INFINITE);
		cout << "Worker Connect!"/* << overlap->sockData->sock_num*/ << endl;

		if (!result || overlap == NULL)
		{
			if (overlap != NULL)
			{
				cout << "Worker socket close" << endl;
				CloseSocket(*(overlap->sockData), true, sockListen == INVALID_SOCKET ? false : true);
			}
		}
		else
		{
			if (overlap->state == RECV)
			{
				overlap->sockData->byteSize = nByteSize;
				if (nByteSize != 0)
					WorkingCommand(*(overlap->sockData));
			}
			else 
				cout<<"State : Send"<<endl;
		}
	}
}

void WorkingCommand(SocketData& sockData)
{
	int localLen, remoteLen;
	sockaddr_in* local = 0, * remote = 0;

	//switch (sockData.recvOverLap.state)
	//{
	//case STATE_READ:
	//case STATE_WRITE:
		if (sockData.byteSize == SOCKET_ERROR)
		{
			cout << "Command sock error socket close" << endl;
			CloseSocket(sockData, true);
		}
		else if (sockData.byteSize == 0)
		{
			cout << "Command size zero socket close" << endl;
			CloseSocket(sockData, false);
		}
		else
		{
			sockData.bufEnd += sockData.byteSize;
			if (DoEcho(sockData))
				PostRead(sockData);
		}
	//	break;
	//}
}

void PostSend(SocketData& sockData)
{
	BOOL result;
	DWORD err;

	sockData.byteSize = sockData.sendData.size();
	sockData.sendBuf.len = sockData.byteSize;
	sockData.sendBuf.buf = const_cast<char*>(sockData.sendData.c_str());
	result = WSASend(sockData.sock, &sockData.sendBuf, 1, &sockData.byteSize, 0, (OVERLAPPED*)&sockData.sendOverLap, 0);

	result = (result != SOCKET_ERROR);
	err = WSAGetLastError();
	if (!result)
	{
		if (err != ERROR_IO_PENDING)
		{
			cout << "Postsend socket close" << endl;
			CloseSocket(sockData, true);
		}
	}
}

void PostRead(SocketData& sockData)
{
	BOOL result;

	if (sockData.bufEnd == &sockData.bufFront[BUF_SIZE])
		sockData.bufEnd = &sockData.bufFront[0];

	sockData.byteSize = &sockData.bufFront[BUF_SIZE] - sockData.bufEnd;
	sockData.recvBuf.len = sockData.byteSize;
	sockData.recvBuf.buf = sockData.bufEnd;
	sockData.recvFlag = 0;
	result = WSARecv(sockData.sock, &sockData.recvBuf, 1, &sockData.byteSize, &sockData.recvFlag, (OVERLAPPED*)&sockData.recvOverLap, 0);

	result = (result != SOCKET_ERROR);
	if (result)
		return;
	else
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			cout << "Postread socket close" << endl;
			CloseSocket(sockData, true);
		}
	}
	return;
}

bool DoEcho(SocketData& sockData)
{
	char buff[BUF_SIZE];
	memcpy(buff, sockData.bufFront, sockData.byteSize);
	buff[sockData.byteSize] = '\0';
	cout << sockData.sock_num << " : " << sockData.bufFront << endl;
	memset(sockData.bufFront, '\0', sizeof(sockData.bufFront));
	sockData.byteSize = 0;
	sockData.bufEnd = &sockData.bufFront[0];
	//Echo인 이유
	iter_vSocketData iter;
	for (iter = vSocketData.begin(); iter != vSocketData.end(); iter++)
	{
		if ((*iter)->recvOverLap.sockData->isConnected )
		{
			(*iter)->sendData = buff;
			PostSend(*(*iter));
		}
	}
	return true;
}

void CloseSocket(SocketData& sockData, bool force, bool listenAgain)
{
	struct linger li = { 0, 0 }; // default: SO_DONTLINGER 
	if (force) li.l_onoff = 1; // SO_LINGER, 	timeout = 0 
	setsockopt(sockData.sock, SOL_SOCKET, SO_LINGER, (char*)&li, sizeof li);
	closesocket(sockData.sock);
	if (listenAgain)
		InitOVERLAP(sockData);
}

void InitOVERLAP(SocketData& sockData)
{
}

int main()
{
	int maxthreads = 4, port = 1500, maxsockets = 4;
	int zero = 0;
	vector<SocketData> vOverlap;
	iter_vSocketData iterOverlap;
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
	//sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons((short)port);
	if (bind(sockListen, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return 0;
	if (listen(sockListen, 200) == SOCKET_ERROR)
		return 0;

	//Create accept Overlap structur
	vSocketData.clear();
	for (int i = 0; i < maxsockets; i++)
	{
		SocketData* sockData = new SocketData(sockListen, i+1);
		//InitOVERLAP(*pOverlap);
		vSocketData.push_back(sockData);
	}

	CreateIoCompletionPort((HANDLE)sockListen, hAcceptIOCP, 0, 0);

	while (TRUE) {};
	for (iterOverlap = vSocketData.begin(); iterOverlap != vSocketData.end(); iterOverlap++)
	{
		closesocket((*iterOverlap)->sock);
		delete (&iterOverlap);
	}
	CloseHandle(hAcceptIOCP); // shut down winsock WSACleanup(); return 0;
	CloseHandle(hWorkerIOCP);
}

