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