#pragma once
#include "Define.h"

struct RequestLoginPacket
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

struct RequestExitChannelPacket
{
	short commandType;
};

struct AnswerExitChannelPacket
{
	short commandType;
	short channelState[MAX_CHANNEL];
	short users[MAX_CHANNEL];
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
	char otherUserName[MAX_USER_NAME];
};

struct NotifyJoinNewUserPacket
{
	short commandType;
	char newUserName[MAX_USER_NAME];
};

struct RequestExitRoomPacket
{
	short commandType;
	char userName[MAX_USER_NAME];
};

struct AnswerExitRoomPacket
{
	short commandType;
	short usingRoomNumSize;
	short usingRoomNum[MAX_ROOM_NUM];
};

struct NotifyDeleteRoomPacket
{
	short commandType;
	short deleteRoomNum;
};

struct NotifyExitRoomPacket
{
	short commandType;
	char userName[MAX_USER_NAME];
};

struct RequestChatRoomPacket
{
	short commandType;
	char userMsg[MAX_MSG_SIZE];
};

struct NotifyChatRoomPacket
{
	short commandType;
	char userName[MAX_USER_NAME];
	char userMsg[MAX_MSG_SIZE];
};