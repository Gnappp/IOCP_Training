#pragma once
#include <iostream> 
#include <WinSock2.h> 
#include <mswsock.h>
#include <string>
#include <vector>
#include <process.h>
#include "Packets.h"
#include "Define.h"

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "mswsock.lib")

using namespace std;

const int addrlen = sizeof(sockaddr_in) + 16;

PlayerData* playerData;
vector<ChannelData> channelData;

bool State_Chating(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped);
bool State_Nameing(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped);
bool State_Channel(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped);

unsigned int __stdcall SendMsg(void* arg) {//전송용 쓰레드함수
	SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
	char msg[BUF_SIZE];
	while (1) {//반복
		cin >> msg;
		if (!strcmp(msg, "q\n") || !strcmp(msg, "Q\n")) {//q를 입력하면 종료한다.
			closesocket(sock);
			send(sock, "", 0, 0);
			exit(0);
		}
		send(sock, msg, strlen(msg), 0);//nameMsg를 서버에게 전송한다.
	}
	return 0;
}

unsigned int __stdcall RecvMsg(void* arg) {
	SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
	char msg[BUF_SIZE] = { 0, };
	short commandType=0;
	short test1=0;
	int strLen;
	while (1) {//반복
		strLen = recv(sock, msg, BUF_SIZE - 1, 0);//서버로부터 메시지를 수신한다.
		//cout << strLen << endl;
		if (strLen == -1)
			return -1;
		CopyMemory(&commandType, msg, sizeof(short));
		CopyMemory(&test1, msg + sizeof(short), sizeof(short));
		cout << test1 << endl;
		cout << msg << endl;
		switch (commandType)
		{
		case ANSWER_LOGIN:
		{
			cout << "Login Success" << endl;
			channelData.clear();

			AnswerLoginPacket* answerLoginPacket = reinterpret_cast<AnswerLoginPacket*>(msg);
			short command = (short)answerLoginPacket->commandType; 
			for (int i = 0; i < MAX_CHANNEL; i++)
			{
				ChannelData ch;

				ZeroMemory(&ch, sizeof(ch));
				channelData.push_back(ch);

				channelData[i].channelState = (short)answerLoginPacket->channelState[i];
				cout << channelData[i].channelState << "  ";
			}
			cout << endl;
			for (int i = 0; i < MAX_CHANNEL; i++)
			{
				channelData[i].users = (short)answerLoginPacket->users[i];
				cout << channelData[i].users << "  ";
			}
			cout << endl;
			cout << answerLoginPacket->userName << endl;
			strcpy_s(playerData->userName, MAX_USER_NAME, answerLoginPacket->userName);
			playerData->isState = STATE_CHANNEL;
			break;
		}
		default:
			msg[strLen] = 0;//문자열의 끝을 알리기 위해 설정
			std::cout << ">>" << msg << '\n';
			break;
		}
	}
	return 0;
}

void main()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		cout << "WSAStartup() error!" << endl;

	SOCKET hSocket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (hSocket == INVALID_SOCKET)
		cout << "socket() error" << endl;;

	SOCKADDR_IN recvAddr;
	memset(&recvAddr, 0, sizeof(recvAddr));
	recvAddr.sin_family = AF_INET;
	recvAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	recvAddr.sin_port = htons(1500);

	if (connect(hSocket, (SOCKADDR*)&recvAddr, sizeof(recvAddr)) == SOCKET_ERROR)
	{
		cout << "connect() error!" << endl;
		return;
	}

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

	playerData = new PlayerData();

	while (true)
	{
		switch (playerData->isState)
		{
		case STATE_NAMEING:
			State_Nameing(dataBuf, hSocket, overlapped);
			break;
		case STATE_CHANNEL:
			State_Channel(dataBuf, hSocket, overlapped);
			break;
		case STATE_WATTING:
			cout << "Waitting" << endl;
			Sleep(500 * 2);
			break;
		case STATE_CHATING:
			State_Chating(dataBuf, hSocket, overlapped);
		}
	}
	closesocket(hSocket);
	WSACleanup();
}

bool State_Nameing(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped)
{
	DWORD dummy = 0;
	char name[MAX_USER_NAME] = { 0, };
	cout << "Name : ";
	cin >> name;
	if (!strcmp(name, "/exit")) return false;

	char tmp_packet[1024] = { 0, };
	TestPacket* testPakcet = (TestPacket*)tmp_packet;
	testPakcet->commandType = (short)REQUEST_LOGIN;
	strcpy_s(testPakcet->userName, MAX_USER_NAME, name);

	dataBuf.len = sizeof(tmp_packet);
	dataBuf.buf = tmp_packet;

	if (WSASend(hSocket, &dataBuf, 1, &dummy, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			cout << "WSASend() error" << endl;
	}
	playerData->isState = STATE_WATTING;
	return true;
}

bool State_Chating(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped)
{
	DWORD dummy = 0;
	char message[1024] = { 0, };
	cout << "Message : ";
	cin >> message;
	if (!strcmp(message, "/exit")) return false;

	dataBuf.len = strlen(message);
	dataBuf.buf = message;
	if (WSASend(hSocket, &dataBuf, 1, &dummy, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			cout << "WSASend() error" << endl;
	}
	return true;
}

bool State_Channel(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped)
{
	DWORD dummy = 0;
	int channelNum = 0;
	cout << "!! Name : " << playerData->userName << " !!" << endl;
	for (int i = 0; i < MAX_CHANNEL; i++)
	{
		cout << i + 1 << " Channel - " << "User : " << channelData[i].users << " State : " << channelData[i].channelState << endl;
	}
	cout << "Select channel : ";
	cin >> channelNum;

	char tmp_packet[1024] = { 0, };
	RequestJoinChannel* testPacket = (RequestJoinChannel*)tmp_packet;
	testPacket->commandType = (short)REQUEST_LOGIN;
	testPacket->channelNum = channelNum;

	dataBuf.len = sizeof(tmp_packet);
	dataBuf.buf = tmp_packet;

	if (WSASend(hSocket, &dataBuf, 1, &dummy, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			cout << "WSASend() error" << endl;
	}
	playerData->isState = STATE_WATTING;
	return true;
}
