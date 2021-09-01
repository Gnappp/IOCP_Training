#include "ChannelData.h"
#include "RoomData.h"
#include "UserData.h"

ChannelData::ChannelData(int channelNum)
{
	this->channelNum=channelNum;
	channelState = 0;
	userDatas.clear();
	roomDatas.clear();
	usingRoomNum.clear();
	for (int i = MAX_ROOM_NUM; i > 0; i--)
	{
		notuseRoomNum.push_back((short)i);
	}
}

short ChannelData::GetChannelState()
{
	int userRate = userDatas.size() / MAX_CHANNEL_USER* 100;

	if (userRate < 30)
	{
		channelState = CHANNEL_USER_LOW;
		return CHANNEL_USER_LOW;
	}
	else if (userRate < 70)
	{
		channelState = CHANNEL_USER_MID;
		return CHANNEL_USER_MID;
	}
	else if (userRate < 99)
	{
		channelState = CHANNEL_USER_HIGH;
		return CHANNEL_USER_HIGH;
	}
	else
	{
		channelState = CHANNEL_USER_FULL;
		return CHANNEL_USER_FULL;
	}
}

short ChannelData::CreateRoom(UserData &userData)
{
	if (notuseRoomNum.size() < 0)
		return 0;
	short useRoomNum = notuseRoomNum[notuseRoomNum.size() - 1];
	notuseRoomNum.pop_back();
	RoomData* newRoomData = new RoomData(useRoomNum, userData, channelNum);
	roomDatas.insert(make_pair(useRoomNum, newRoomData));
	usingRoomNum.insert(useRoomNum);
	return useRoomNum;
}

bool ChannelData::JoinRoom(short roomNum, UserData &userData)
{
	map<int, RoomData*>::iterator iter = roomDatas.find(roomNum);
	return iter->second->JoinRoom(roomNum, userData);
}

bool ChannelData::DeleteRoom(short roomNum)
{
	if (roomDatas.find(roomNum)->second->userDatas.size() != 0)
		return false;
	roomDatas.erase(roomDatas.find(roomNum));
	usingRoomNum.erase(roomNum);
	notuseRoomNum.push_back(roomNum);
	return true;
}