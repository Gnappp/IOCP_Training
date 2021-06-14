#include "RoomData.h"

RoomData::RoomData(short newRoomNum, UserData& userData, short iChannelNum)
{
	roomNum = 0;
	userDatas.clear();
	channelNum = 0;
	isFull = false;

	roomNum = newRoomNum;
	userDatas.insert(make_pair(userData.userIndex, &userData));
	channelNum = iChannelNum;
}

bool RoomData::JoinRoom(short iRoomNum, UserData& joinUserData)
{
	if (roomNum != iRoomNum)
		return false;

	if (userDatas.size() < 2 && !isFull)
	{
		userDatas.insert(make_pair(joinUserData.userIndex, &joinUserData));
		isFull = true;
		return true;
	}
	else
		return false;
}

void RoomData::ExitRoom(short iRoomNum, UserData& exitUserIndex)
{
	if (roomNum != iRoomNum)
		return;

	map<int, UserData*>::iterator iter_findUser = userDatas.find(exitUserIndex.userIndex);
	if (iter_findUser != userDatas.end())
	{
		userDatas.erase(userDatas.find(exitUserIndex.userIndex));
		isFull = false;
	}
}