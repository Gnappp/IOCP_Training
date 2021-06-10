#pragma once
#include <vector>
#include <map>

#include "UserData.h"
#include "Define.h"

using namespace std;

class RoomData
{
public:
	short roomNum;
	map<int, UserData*> userDatas; // map으로 관리해주어야한다.
	short channelNum;
	bool isFull;

	RoomData(short newRoomNum, UserData& userData, short channelNum);
	bool JoinRoom(short iRoomNum, UserData& joinUserIndex);
};