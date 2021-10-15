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

struct UserPacketData
{
	char userPacket[BUF_SIZE];
	UserPacketData()
	{
		ZeroMemory(userPacket, BUF_SIZE);
	}
};


class SocketData
{
public:
	BOOL isConnected;
	SOCKET sock;
	DWORD byteSize; //���۹���Ʈ
	char bufRecvData[BUF_SIZE]; //recv�ϸ� ���⿡ �����Ͱ� ����
	
	sockaddr_in mLocal, mRemote;
	char* sendData;
	WSABUF recvBuf, sendBuf;
	DWORD recvFlag;
	stOverlap recvOverLap, sendOverLap;
	UserData* userData;
	int userSockIndex;
	queue<UserPacketData> userPacketQueue;

	SocketData(SOCKET& listenSock, int iSock_num);

	~SocketData() { cout << "ending TT" << endl; }
};
