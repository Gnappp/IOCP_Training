#pragma once
#include <mswsock.h>
#include <string>
#include <vector>
#include <process.h>
#include <iostream> 
#include <WinSock2.h> 
#include <string>
#include "Define.h"
#include "UserData.h"
#include "RoomData.h"
#include "ChannelData.h"

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "mswsock.lib")

using namespace std;

class SocketData;

vector<ChannelData> ch;

HANDLE hAcceptIOCP;
HANDLE hWorkerIOCP;
SOCKET sockListen = INVALID_SOCKET;

typedef vector<SocketData*> vecSocketData;
typedef vecSocketData::iterator iter_vSocketData;
vecSocketData vSocketData; //소켓 데이터들

enum SOCKET_STATE
{
	RECV,SEND
};

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
	char bufRecvData[BUF_SIZE];
	char* bufEnd;
	sockaddr_in mLocal, mRemote;
	char* sendData;
	WSABUF recvBuf, sendBuf;
	DWORD recvFlag;
	stOverlap recvOverLap,sendOverLap;
	int userIndex;
	UserData* userData;

	SocketData(SOCKET& listenSock,int iSock_num);

	~SocketData() { cout << "ending TT" << endl; }
};


void CloseSocket(SocketData& overlap, bool force = false, bool listenAgain = true);
void WorkingCommand(SocketData& overlap);
void PostRead(SocketData& overlap);
void InitOVERLAP(SocketData& overlap);
bool DoEcho(SocketData& overlap);
void CloseSocket(SocketData& overlap, bool force, bool listenAgain);


