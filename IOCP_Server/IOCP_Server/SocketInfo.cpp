#include "SocketInfo.h"
#include "UserData.h"
#include <iostream>
#include "AllLib.h"
//#include <WinSock2.h> 
 
// 초기화할때 AcceptEx를 이용하여 리슨될때 생성한 소켓으로 갈 수 있게 해준다.
// AcceptEx(리슨소켓, 만들어진넣을소켓, 수신된 데이터를 받을 버퍼[char []],	받을 데이터크기[int], 소켓 ip크기[int], 소켓 ip크기, 받은데이터 크기[DWORD], overlap 구조체
// GetAcceptExSockaddrs(overlap의 버퍼크기,	받을 데이터크기, 소켓 ip크기, 소켓 ip크기, 로컬ip받을 sockaddr_in, 로컬 ip 크기[int], 리모트 ip받을 sockaddr_in, 리모트 ip 크기[int])

SocketData::SocketData(SOCKET& listenSock, int iSock_num)
{
	memset(this, NULL, sizeof(*this));
	int zero = 0;
	isConnected = false;
	ZeroMemory(bufRecvData, BUF_SIZE);
	ZeroMemory(&recvBuf, sizeof(WSABUF));
	sendData = { 0, };
	if (userData != NULL)
		free(userData);
	userData = new UserData(iSock_num);
	userSockIndex = iSock_num;

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		cout << "Fail create sock" << endl;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero));
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero));

	sendOverLap.state = SEND;
	recvOverLap.state = RECV;
	recvOverLap.sockData = this;
	AcceptEx(listenSock, sock, &bufRecvData[0], 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &byteSize, (OVERLAPPED*)&recvOverLap);
}