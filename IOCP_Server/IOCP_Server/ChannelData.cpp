#include "ChannelData.h"

ChannelData::ChannelData(int channelNum)
{
	this->channelNum=channelNum;
	userDatas.clear();
	roomDatas.clear();
}

short ChannelData::GetChannelState()
{
	int userRate = userDatas.size() / MAX_CHANNEL_USER* 100;

	if (userRate < 30)
		return CHANNEL_USER_LOW;
	else if (userRate < 70)
		return CHANNEL_USER_MID;
	else if (userRate < 99)
		return CHANNEL_USER_HIGH;
	else
		return CHANNEL_USER_FULL;
}