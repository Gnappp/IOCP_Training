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

// ������ �뵵
enum SOCKET_STATE
{
	RECV, SEND
};

// OVERLAPPED�� ����Ͽ� �������� �߰�
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
	DWORD byteSize; // recv�� ���� ����Ʈ��
	char bufRecvData[BUF_SIZE]; // recv�� �ޱ����� ���ڿ�
	
	sockaddr_in mLocal, mRemote;
	char* sendData;
	WSABUF recvBuf, sendBuf;
	DWORD recvFlag;
	stOverlap recvOverLap, sendOverLap; //recv, send ����
	UserData* userData; 
	int userSockIndex; // ���Ϲ�ȣ

	SocketData(SOCKET& listenSock, int iSock_num);

	~SocketData() { cout << "ending TT" << endl; }
};
