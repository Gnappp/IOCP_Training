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
	map<int,UserData*> userDatas; // 채널의 유저시리얼 관리
	map<int, RoomData*> roomDatas;
	set<short> usingRoomNum;
	vector<short> notuseRoomNum; 

	ChannelData(int channelNum);
	short GetChannelState();

	short CreateRoom(UserData& userData);
	bool JoinRoom(short roomNum, UserData& userData);
	bool DeleteRoom(short roomNum);
};