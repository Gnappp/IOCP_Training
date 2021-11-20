#pragma once
#include "Define.h"

#include <set>
#include <map>
#include <vector>
#include <iostream>
using namespace std;

class UserData;
class RoomData;

class ChannelData
{
public:
	int channelState;
	int channelNum;
	map<int,UserData*> userDatas; // 盲澄狼 蜡历 包府
	map<int, RoomData*> roomDatas; // 积己等 规狼 包府

	//规锅龋甫 包府
	set<short> usingRoomNum; 
	vector<short> notuseRoomNum; 

	ChannelData(int channelNum);
	short GetChannelState();

	short CreateRoom(UserData& userData);
	bool JoinRoom(short roomNum, UserData& userData);
	bool DeleteRoom(short roomNum);
};