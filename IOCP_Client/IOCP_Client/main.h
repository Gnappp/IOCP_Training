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

class ChannelData
{
public:
	short channelNum;
	set<short> roomNums;
	ChannelData()
	{
		channelNum = 0;
		roomNums.clear();
	}
};

class PlayerData
{
public:
	short isState;
	char userName[MAX_USER_NAME];
	short channelNum;
	short roomNum;
	PlayerData()
	{
		isState = STATE_NAMEING;
		ZeroMemory(userName, MAX_USER_NAME);
	}
};

PlayerData* playerData;
vector<PlayerData*> otherPlayer;
ChannelData* channelData;

bool State_Chating(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped);
bool State_Nameing(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped);
bool State_Channel(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped);
bool State_Channel_In(WSABUF& dataBuf, SOCKET& hSocket, OVERLAPPED& overlapped);
