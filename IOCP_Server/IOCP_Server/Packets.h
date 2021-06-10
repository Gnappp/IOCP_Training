#pragma once
#include "Define.h"

struct TestPacket
{
	short commandType;
	char userName[MAX_USER_NAME];
};

struct AnswerLoginPacket
{
	short commandType;
	short channelState[MAX_CHANNEL];
	short users[MAX_CHANNEL];
	char userName[MAX_USER_NAME];
};

struct RequestJoinChannelPacket
{
	short commandType;
	short channelNum;
};

struct AnswerJoinChannelPacket
{
	short commandType;
	bool isSuccess;
	short usingRoomNumSize;
	short usingRoomNum[MAX_ROOM_NUM];
};

struct RequestCreateRoomPacket
{
	short commandType;
	short channelNum;
};

struct AnswerCreateRoomPacket
{
	short commandType;
	bool isCreate;
	short roomNum;
};

struct NotifyCreateRoomPacket
{
	short commandType;
	short roomNum;
};

struct RequsetJoinRoomPacket
{
	short commandType;
	short roomNum;
};

struct AnswerJoinRoomPacket
{
	short commandType;
	bool isJoin;
	short roomNum;
	short userNum;
	short otherUserIndex;
	char otherUserName[MAX_USER_NAME];
};

struct NotifyJoinNewPlayerPacket
{
	short commandType;
	char newPlayerName[MAX_USER_NAME];
};

struct RequestChatRoom
{
	short commandType;
	char userMsg[MAX_MSG_SIZE];
};

struct NotifyChatRoom
{
	short commandType;
	char userName[MAX_USER_NAME];
	char userMsg[MAX_MSG_SIZE];
};