#pragma once
#include "UserData.h"
#include "RoomData.h"

#include<set>
#include<iostream>
using namespace std;

class ChannelData
{
public:
	int channelNum;
	set<int> userDatas;
	set<RoomData> roomDatas;

	ChannelData(int channelNum);
	short GetChannelState();
};