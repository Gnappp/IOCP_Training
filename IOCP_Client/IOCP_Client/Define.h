#pragma once
const int BUF_SIZE = 1024;
const int MAX_USER_NAME = 20;
const int MAX_CHANNEL = 2;

enum PLAYER_STATE
{
	STATE_NAMEING=1, STATE_WATTING, STATE_CHATING,STATE_CHANNEL
};

enum COMMAND_TYPE
{
	REQUEST_LOGIN = 1, ANSWER_LOGIN, REQUEST_JOIN_CHANNEL
};

enum CANNEL_STATE
{
	CHANNEL_USER_LOW=1, CHANNEL_USER_MID, CHANNEL_USER_HIGH, CHANNEL_USER_FULL
};

class PlayerData
{
public:
	short isState;
	char userName[MAX_USER_NAME];

	PlayerData()
	{
		isState = STATE_NAMEING;
		ZeroMemory(userName, MAX_USER_NAME);
	}
};

struct ChannelData
{
	int users;
	short channelState;
};