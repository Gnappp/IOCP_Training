#include "SocketInfo.h"
#include "UserData.h"
#include <iostream>
#include "AllLib.h"
//#include <WinSock2.h> 
 
// �ʱ�ȭ�Ҷ� AcceptEx�� �̿��Ͽ� �����ɶ� ������ �������� �� �� �ְ� ���ش�.
// AcceptEx(��������, ���������������, ���ŵ� �����͸� ���� ����[char []],	���� ������ũ��[int], ���� ipũ��[int], ���� ipũ��, ���������� ũ��[DWORD], overlap ����ü
// GetAcceptExSockaddrs(overlap�� ����ũ��,	���� ������ũ��, ���� ipũ��, ���� ipũ��, ����ip���� sockaddr_in, ���� ip ũ��[int], ����Ʈ ip���� sockaddr_in, ����Ʈ ip ũ��[int])

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