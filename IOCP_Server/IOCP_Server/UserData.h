#pragma once
#include "Define.h"

#include<iostream>

class UserData
{
public:
	int winCount, loseCount, DrawCount;
	short isWhere;
	char userName[MAX_USER_NAME];
	short channelNum;
	short roomNum;
	int userIndex;
	int userSockIndex;

	UserData(int userSock);
};