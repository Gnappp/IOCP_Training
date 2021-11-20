#pragma once
#include <map>
#include <functional>
#include <windows.h>

enum IS_WHERE
{
	CHANNEL_SELECT=1, ROOM_SELECT, ROOM_IN,
};

enum COMMAND_TYPE
{
	REQUEST_LOGIN = 1, ANSWER_LOGIN,
	REQUEST_JOIN_CHANNEL, ANSWER_JOIN_CHANNEL, REQUEST_EXIT_CHANNEL, ANSWER_EXIT_CHANNEL,
	REQUEST_CREATE_ROOM, ANSWER_CREATE_ROOM, NOTIFY_CREATE_ROOM, NOTIFY_DELETE_ROOM,
	REQUEST_JOIN_ROOM, ANSWER_JOIN_ROOM, NOTIFY_JOIN_NEW_PLAYER,
	REQUEST_EXIT_ROOM, ANSWER_EXIT_ROOM, NOTIFY_EXIT_ROOM,
	REQUEST_CHAT_ROOM, NOTIFY_CHAT_ROOM
};

enum CANNEL_STATE
{
	CHANNEL_USER_LOW=1, CHANNEL_USER_MID, CHANNEL_USER_HIGH, CHANNEL_USER_FULL
};

const int MAX_USER_NAME = 20;
const int BUF_SIZE = 1024;
const int MAX_SOCKET = 10;
const int MAX_CHANNEL = 2;
const int MAX_CHANNEL_USER = 30;
const int MAX_ROOM_NUM = 50;
const int MAX_MSG_SIZE = 900;

class SocketData;
// 유저 패킷큐에서 사용할 구조체
struct UserPacketData
{
	char userPacket[BUF_SIZE];
	SocketData* userSocketData;
	UserPacketData()
	{
		ZeroMemory(userPacket, BUF_SIZE);
	}
};

struct DbPacketData
{
	char qry[BUF_SIZE];
	short qryType;
	SocketData* userSocketData;
	DbPacketData()
	{
		ZeroMemory(qry, BUF_SIZE);
	}
};