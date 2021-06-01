#pragma once
#include <iostream> 
#include <WinSock2.h> 
#include <mswsock.h>
#include <string>
#include <vector>
#include <process.h>

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "mswsock.lib")

using namespace std;

void PostRecv();

const int BUF_SIZE = 1024;
const int addrlen = sizeof(sockaddr_in) + 16;

unsigned int __stdcall SendMsg(void* arg) {//���ۿ� �������Լ�
	SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
	char msg[BUF_SIZE];
	while (1) {//�ݺ�
		cin >> msg;
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {//q�� �Է��ϸ� �����Ѵ�.
			closesocket(sock);
			send(sock, "", 0, 0);
			exit(0);
		}
		send(sock, msg, strlen(msg), 0);//nameMsg�� �������� �����Ѵ�.
	}
	return 0;
}

unsigned int __stdcall RecvMsg(void* arg) {
	SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
	char msg[BUF_SIZE];
	int strLen;
	while (1) {//�ݺ�
		strLen = recv(sock, msg, BUF_SIZE - 1, 0);//�����κ��� �޽����� �����Ѵ�.
		if (strLen == -1)
			return -1;
		msg[strLen] = 0;//���ڿ��� ���� �˸��� ���� ����
		std::cout << ">>" << msg << '\n';
	}
	return 0;
}

void main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		cout<<"WSAStartup() error!"<<endl;

	SOCKET hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		cout << "socket() error" << endl;;

	SOCKADDR_IN recvAddr;
	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	recvAddr.sin_port = htons(1500);

	if (connect(hSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
		cout << "connect() error!" << endl;

	WSAEVENT event = WSACreateEvent();

	OVERLAPPED overlapped;
	memset(&overlapped, NULL, sizeof(overlapped));

	overlapped.hEvent = event;

	WSABUF dataBuf;
	ZeroMemory(&dataBuf, sizeof(dataBuf));
	char message[1024] = { 0, };
	int sendBytes = 0;
	int recvBytes = 0;
	int flags = 0;

	HANDLE recvThread = (HANDLE)_beginthreadex(NULL, 0, RecvMsg, (void*)&hSocket, 0, NULL);
	cout << "Connected!" << endl;

	while (true)
	{
		DWORD dummy = 0;
		char message[1024] = { 0, };
		cout << "message : ";
		cin >> message;
		if (!strcmp(message, "/exit")) break;

		dataBuf.len = strlen(message);
		dataBuf.buf = message;
		if (WSASend(hSocket, &dataBuf, 1, &dummy, 0, &overlapped, NULL) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
				cout<<"WSASend() error"<<endl;
		}
	}
	closesocket(hSocket);
	WSACleanup();
}
