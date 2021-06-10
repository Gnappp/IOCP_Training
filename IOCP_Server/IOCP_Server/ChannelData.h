#pragma once
#include "UserData.h"
#include "RoomData.h"
#include "Define.h"

#include <set>
#include <map>
#include <vector>
#include <iostream>
using namespace std;


class ChannelData
{
public:
	short channelState;
	short channelNum;
	map<int, UserData*> userDatas; // map���� ���� �����ؾ��ҵ�
	set<short> usingRoomNum;
	vector<short> notuseRoomNum; 
	map<int, RoomData*> roomDatas;

	ChannelData(int channelNum);
	short GetChannelState();

	short CreateRoom(UserData& userData);
	bool JoinRoom(short roomNum, UserData& userData);
};