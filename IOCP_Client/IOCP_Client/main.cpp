#pragma once
#include "main.h"

using namespace std;

const int addrlen = sizeof(sockaddr_in) + 16;

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
		if (strLen == -1)
			return -1;
		CopyMemory(&commandType, msg, sizeof(short));
		CopyMemory(&test1, msg + sizeof(short), sizeof(short));
		switch (commandType)
		{
		case ANSWER_LOGIN:
		{
			cout << "Login Success" << endl;

			AnswerLoginPacket* recvPacket = reinterpret_cast<AnswerLoginPacket*>(msg);
			short command = (short)recvPacket->commandType;

			cout << endl;
			cout << "!! Name : " << recvPacket->userName << " !!" << endl;
			for (int i = 0; i < MAX_CHANNEL; i++)
			{
				cout << i + 1 << " Channel - " << "User : " << (short)recvPacket->users[i]
					<< " State : " << (short)recvPacket->channelState[i] << endl;
			}
			cout << endl;
			cout << recvPacket->userName << endl;
			strcpy_s(playerData->userName, MAX_USER_NAME, recvPacket->userName);
			playerData->isState = STATE_CHANNEL;
			break;
		}
		case ANSWER_JOIN_CHANNEL:
		{
			cout << "Channel in" << endl;
			AnswerJoinChannelPacket* recvPacket = reinterpret_cast<AnswerJoinChannelPacket*>(msg);
			short command = (short)recvPacket->commandType;
			if (recvPacket->isSuccess)
			{
				int usingRoomNum = (short)recvPacket->usingRoomNumSize;
				if (usingRoomNum != 0)
				{
					for (int i = 0; i < usingRoomNum; i++)
					{
						cout << "Room " << recvPacket->usingRoomNum[i] << endl;
						channelData->roomNums.insert(recvPacket->usingRoomNum[i]);
					}
				}
				else
				{
					cout << "No room" << endl;
				}
				playerData->isState = STATE_CHANNEL_IN;
			}
			else
				playerData->isState = STATE_CHANNEL;
			break;
		}
		case ANSWER_CREATE_ROOM:
		{
			AnswerCreateRoomPacket* recvPacket = reinterpret_cast<AnswerCreateRoomPacket*>(msg);
			if (!recvPacket->isCreate)
				break;
			system("cls");
			cout << "Room in" << endl;
			playerData->roomNum = recvPacket->roomNum;
			playerData->isState = STATE_ROOM;
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


bool State_Nameing(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped)
{
	DWORD dummy = 0;
	char name[MAX_USER_NAME] = { 0, };
	cout << "Name : ";
	cin >> name;
	if (!strcmp(name, "/exit")) return false;

	char tmp_packet[1024] = { 0, };
	TestPacket* sendPacket = (TestPacket*)tmp_packet;
	sendPacket->commandType = (short)REQUEST_LOGIN;
	strcpy_s(sendPacket->userName, MAX_USER_NAME, name);

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
	short channelNum = 0;
	cout << "Select channel : ";
	cin >> channelNum;
	channelData->channelNum = channelNum-1;
	playerData->channelNum = channelNum - 1;

	char tmp_packet[1024] = { 0, };
	RequestJoinChannelPacket* sendPacket = (RequestJoinChannelPacket*)tmp_packet;
	sendPacket->commandType = (short)REQUEST_JOIN_CHANNEL;
	sendPacket->channelNum = channelNum - 1;

	dataBuf.len = sizeof(RequestJoinChannelPacket);
	dataBuf.buf = tmp_packet;

	if (WSASend(hSocket, &dataBuf, 1, &dummy, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			cout << "WSASend() error" << endl;
	}
	playerData->isState = STATE_WATTING;
	return true;
}

bool State_Channel_In(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped)
{
	DWORD dummy = 0;
	short commandNum = 0;
	if (channelData->roomNums.size() != 0)
		cout << "1. Join room" << endl;
	cout << "2. Create room" << endl;
	cout << ">> ";
	cin >> commandNum;

	
	if (commandNum == 1)
	{
		if (channelData->roomNums.size() == 0)
			return false;
		cout << "Room No >> ";
		cin >> commandNum;
		char tmp_packet[1024] = { 0, };
		RequsetJoinRoomPacket* sendPacket = (RequsetJoinRoomPacket*)tmp_packet;
		sendPacket->commandType = REQUEST_JOIN_ROOM;
		sendPacket->roomNum = commandNum;

		dataBuf.len = sizeof(RequsetJoinRoomPacket);
		dataBuf.buf = tmp_packet;
	}

	else if (commandNum == 2)
	{
		char tmp_packet[1024] = { 0, };
		RequestCreateRoomPacket* sendPacket = (RequestCreateRoomPacket*)tmp_packet;
		sendPacket->commandType = (short)REQUEST_CREATE_ROOM;
		sendPacket->channelNum = playerData->channelNum;

		dataBuf.len = sizeof(RequestCreateRoomPacket);
		dataBuf.buf = tmp_packet;
	}
	
	if (WSASend(hSocket, &dataBuf, 1, &dummy, 0, &overlapped, NULL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			cout << "WSASend() error" << endl;
	}

	playerData->isState = STATE_WATTING;
	return true;
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
	channelData = new ChannelData();
	otherPlayer.clear();

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
			break;
		case STATE_CHANNEL_IN:
			State_Channel_In(dataBuf, hSocket, overlapped);
			break;
		case STATE_ROOM:
			break;
		}
	}
	closesocket(hSocket);
	WSACleanup();
}
