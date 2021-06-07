#pragma once
#include "Define.h"
#include "RoomData.h"

#include<iostream>

class UserData
{
public:
	int winCount, loseCount, DrawCount;
	int isWhere;
	char userName[MAX_USER_NAME];
	int channalNum;
	int roomNum;

	UserData();
};