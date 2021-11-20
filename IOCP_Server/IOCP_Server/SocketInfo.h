#pragma once
#include "AllLib.h"
#include "Define.h"

#include <iostream> 
#include <string>
#include <process.h>
#include <queue>

using namespace std;

class UserData; 
class SocketData;

// 소켓의 용도
enum SOCKET_STATE
{
	RECV, SEND
};

// OVERLAPPED를 사용하여 소켓정보 추가
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
	DWORD byteSize; // recv시 받은 바이트수
	char bufRecvData[BUF_SIZE]; // recv시 받기위한 문자열
	
	sockaddr_in mLocal, mRemote;
	char* sendData;
	WSABUF recvBuf, sendBuf;
	DWORD recvFlag;
	stOverlap recvOverLap, sendOverLap; //recv, send 관리
	UserData* userData; 
	int userSockIndex; // 소켓번호

	SocketData(SOCKET& listenSock, int iSock_num);

	~SocketData() { cout << "ending TT" << endl; }
};
