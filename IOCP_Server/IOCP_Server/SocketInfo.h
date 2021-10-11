#pragma once
#include <iostream> 
#include <WinSock2.h> 
#include <mswsock.h>
#include <string>
#include <process.h>
#include <queue>
#include "Define.h"

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "mswsock.lib")

using namespace std;

class UserData;

enum SOCKET_STATE
{
	RECV, SEND
};

class SocketData;
typedef struct stOverlap
{
	OVERLAPPED m_overlapped;
	int state;
	SocketData* sockData;
}stOverlap;

class SocketData
{
public:
	BOOL isConnected;
	SOCKET sock;
	DWORD byteSize; //전송바이트
	char bufRecvData[BUF_SIZE]; //recv하면 여기에 데이터가 들어옴
	
	sockaddr_in mLocal, mRemote;
	char* sendData;
	WSABUF recvBuf, sendBuf;
	DWORD recvFlag;
	stOverlap recvOverLap, sendOverLap;
	UserData* userData;
	int userSockIndex;
	//queue<UserPacketData> userPacketQueue;
	char userPacketQueue[BUF_SIZE * 2];
	queue<int> userPacketQueueSize;
	char* queueFront;
	char* queueEnd;

	SocketData(SOCKET& listenSock, int iSock_num);

	~SocketData() { cout << "ending TT" << endl; }
};
