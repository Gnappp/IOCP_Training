#pragma once
#include <mswsock.h>
#include <string>
#include <vector>
#include <process.h>
#include <iostream> 
#include <WinSock2.h> 
#include <string>
#include <map>

#include "Define.h"
#include "UserData.h"
#include "RoomData.h"
#include "ChannelData.h"

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "mswsock.lib")

using namespace std;

vector<ChannelData> ch;
typedef map<int, UserData> USER_DATA;
USER_DATA mUserDatas;

HANDLE hAcceptIOCP;
HANDLE hWorkerIOCP;
SOCKET sockListen = INVALID_SOCKET;

enum SOCKET_STATE
{
	RECV,SEND
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
	char* bufEnd;
	sockaddr_in mLocal, mRemote;
	char* sendData;
	WSABUF recvBuf, sendBuf;
	DWORD recvFlag;
	stOverlap recvOverLap,sendOverLap;
	UserData* userData;
	int userSockIndex;

	SocketData(SOCKET& listenSock,int iSock_num);

	~SocketData() { cout << "ending TT" << endl; }
};


typedef vector<SocketData*> vecSocketData;
typedef vecSocketData::iterator iter_vSocketData;
vecSocketData vSocketData; //소켓 데이터들

void CloseSocket(SocketData& overlap, bool force = false, bool listenAgain = true);
void WorkingCommand(SocketData& overlap);
void PostRead(SocketData& overlap);
void InitOVERLAP(SocketData& overlap);
bool DoEcho(SocketData& overlap);
void CloseSocket(SocketData& overlap, bool force, bool listenAgain);


