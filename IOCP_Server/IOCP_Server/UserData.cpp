#include "UserData.h"

UserData::UserData(int userSock)
{
	memset(this, NULL, sizeof(*this));
	userSockIndex = userSock;
}