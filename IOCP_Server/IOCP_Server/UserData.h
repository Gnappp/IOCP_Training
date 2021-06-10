#pragma once
#include "Define.h"

#include<iostream>

class UserData
{
public:
	int winCount, loseCount, DrawCount;
	short isWhere;
	char userName[MAX_USER_NAME];
	short channalNum;
	short roomNum;
	int userIndex;
	int userSockIndex;

	UserData(int userSock);
};