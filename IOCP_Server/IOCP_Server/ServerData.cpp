#pragma once
#include "ServerData.h"
#include "SocketInfo.h"
#include "IOCP_main.h"
#include "ThreadPool.h"
#include "ChannelData.h"
#include "RoomData.h"
#include "Packets.h"
#include "UserData.h"

IocpMain* iocpMain;

unsigned int __stdcall AcceptThread(void* arg)
{
	DWORD nByteSize, key;
	BOOL result;
	string str;
	stOverlap* overlap;
	int localLen, remoteLen;
	sockaddr_in* local = 0, * remote = 0;

	while (1)
	{
		result = GetQueuedCompletionStatus(iocpMain->GetInstance()->hAcceptIOCP, &nByteSize, (PULONG_PTR)&key, (OVERLAPPED**)&overlap, INFINITE);
		cout << "Accept Connect!" << endl;

		if (!result || overlap == NULL)
		{
			if (overlap != NULL)
			{
				cout << "Accept socket close" << endl;
				CloseSocket(*(overlap->sockData), true, iocpMain->GetInstance()->sockListen == INVALID_SOCKET ? false : true);
			}
		}
		else
		{
			if (!overlap->sockData->isConnected)
			{
				GetAcceptExSockaddrs(&overlap->sockData->bufRecvData[0], 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr**)&local, &localLen, (sockaddr**)&remote, &remoteLen);
				memcpy(&overlap->sockData->mLocal, local, sizeof(sockaddr_in));
				memcpy(&overlap->sockData->mRemote, remote, sizeof(sockaddr_in));
				setsockopt(overlap->sockData->sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&iocpMain->GetInstance()->sockListen, sizeof(iocpMain->GetInstance()->sockListen));
				overlap->sockData->isConnected = true;

				CreateIoCompletionPort((HANDLE)overlap->sockData->sock, iocpMain->GetInstance()->hWorkerIOCP, (DWORD)overlap->sockData, 0);
				PostRead(*(overlap->sockData));
			}
		}
	}
}

unsigned int __stdcall WorkerThread(void* arg)
{
	DWORD nByteSize, key;
	BOOL result;
	stOverlap* overlap;

	while (1)
	{
		result = GetQueuedCompletionStatus(iocpMain->GetInstance()->hWorkerIOCP, &nByteSize, (PULONG_PTR)&key, (OVERLAPPED**)&overlap, INFINITE);
		if (!result || overlap == NULL)
		{
			if (overlap != NULL)
			{
				cout << "Worker socket close" << endl;
				CloseSocket(*(overlap->sockData), true, iocpMain->GetInstance()->sockListen == INVALID_SOCKET ? false : true);
			}
		}
		else
		{
			if (overlap->state == RECV)
			{
				overlap->sockData->byteSize = nByteSize;
				if (nByteSize != 0)
					WorkingCommand(*(overlap->sockData));
				PostRead(*(overlap->sockData));
			}
		}
	}
}

void WorkingCommand(SocketData& sockData)
{
	int localLen, remoteLen;
	sockaddr_in* local = 0, * remote = 0;

	if (sockData.byteSize == SOCKET_ERROR)
	{
		cout << "Command sock error socket close" << endl;
		CloseSocket(sockData, true);
	}
	else if (sockData.byteSize == 0)
	{
		cout << "Command size zero socket close" << endl;
		CloseSocket(sockData, false);
	}
	else
	{
		UserPacketEnqueue(sockData);
		
		ResetRecvBuff(sockData);
		PostRead(sockData);
	}
}

bool UserPacketEnqueue(SocketData& sockData)
{
	UserPacketData* tmp_PacketData = new UserPacketData;
	memcpy(tmp_PacketData->userPacket, sockData.bufRecvData, BUF_SIZE);
	tmp_PacketData->userSocketData = &sockData;
	iocpMain->GetInstance()->threadPool->EnqueueWork(tmp_PacketData);
	return true;
}

void PostSend(SocketData& sockData)
{
	BOOL result;
	DWORD err;

	sockData.sendBuf.len = sockData.byteSize;
	sockData.sendBuf.buf = sockData.sendData;
	result = WSASend(sockData.sock, &sockData.sendBuf, 1, &sockData.byteSize, 0, (OVERLAPPED*)&sockData.sendOverLap, 0);

	result = (result != SOCKET_ERROR);
	err = WSAGetLastError();
	if (!result)
	{
		if (err != ERROR_IO_PENDING)
		{
			cout << "Postsend socket close" << endl;
			CloseSocket(sockData, true);
		}
	}
}

void PostRead(SocketData& sockData)
{
	BOOL result;

	sockData.recvBuf.len = BUF_SIZE;
	sockData.recvBuf.buf = &sockData.bufRecvData[0];
	sockData.recvFlag = 0;
	result = WSARecv(sockData.sock, &sockData.recvBuf, 1, &sockData.byteSize, &sockData.recvFlag, (OVERLAPPED*)&sockData.recvOverLap, 0);

	result = (result != SOCKET_ERROR);
	if (result)
		return;
	else
	{
		if (GetLastError() != ERROR_IO_PENDING)
		{
			cout << "Postread socket close" << endl;
			CloseSocket(sockData, true);
		}
	}
	return;
}

void CloseSocket(SocketData& sockData, bool force, bool listenAgain)
{
	struct linger li = { 0, 0 }; // default: SO_DONTLINGER 
	if (force) li.l_onoff = 1; // SO_LINGER, 	timeout = 0 
	setsockopt(sockData.sock, SOL_SOCKET, SO_LINGER, (char*)&li, sizeof li);
	closesocket(sockData.sock);
	if (listenAgain)
		InitOVERLAP(sockData);
}

void InitOVERLAP(SocketData& sockData)
{
}
void ResetRecvBuff(SocketData& sockData)
{
	memset(sockData.bufRecvData, '\0', sizeof(sockData.bufRecvData));
	sockData.byteSize = 0;
}

void RequestLogin(UserPacketData* userPacketData)
{
	SocketData* sockData = userPacketData->userSocketData;
	RequestLoginPacket* packet = reinterpret_cast<RequestLoginPacket*>(userPacketData->userPacket);
	strcpy_s(sockData->userData->userName, MAX_USER_NAME, packet->userName);
	cout << sockData->userSockIndex << " : " << packet->userName << endl;

	sockData->userData->userIndex = rand();
	if (iocpMain->GetInstance()->mUserDatas.find(sockData->userData->userIndex) != iocpMain->GetInstance()->mUserDatas.end())
	{
		sockData->userData->userIndex = rand();
		while (1)
		{
			if (iocpMain->GetInstance()->mUserDatas.find(sockData->userData->userIndex) != iocpMain->GetInstance()->mUserDatas.end())
				sockData->userData->userIndex = rand();
			else
				break;
		}
	}
	iocpMain->GetInstance()->mUserDatas.insert(make_pair(sockData->userData->userIndex, sockData->userData));
	sockData->userData->isWhere = CHANNEL_SELECT;

	char tmp_packet[1024] = { 0, };
	AnswerLoginPacket* sendPacket = (AnswerLoginPacket*)tmp_packet;
	sendPacket->commandType = (short)ANSWER_LOGIN;
	for (int i = 0; i < MAX_CHANNEL; i++)
	{
		sendPacket->channelState[i] = (short)iocpMain->GetInstance()->ch[i]->GetChannelState();
	}
	for (int i = 0; i < MAX_CHANNEL; i++)
	{
		sendPacket->users[i] = (short)iocpMain->GetInstance()->ch[i]->userDatas.size();
	}
	CopyMemory(sendPacket->userName, sockData->userData->userName, strlen(sockData->userData->userName));
	sockData->sendData = tmp_packet;
	sockData->byteSize = sizeof(AnswerLoginPacket);
	PostSend(*sockData);
}

void RequestJoinChannel(UserPacketData* userPacketData)
{
	SocketData* sockData = userPacketData->userSocketData;	
	RequestJoinChannelPacket* recvPacket = reinterpret_cast<RequestJoinChannelPacket*>(userPacketData->userPacket);

	char tmp_packet[1024] = { 0, };
	AnswerJoinChannelPacket* sendPacket = (AnswerJoinChannelPacket*)tmp_packet;
	sendPacket->commandType = (short)ANSWER_JOIN_CHANNEL;
	cout << "Channel No " << (short)recvPacket->channelNum << endl;

	if (iocpMain->GetInstance()->ch[recvPacket->channelNum]->GetChannelState() != CHANNEL_USER_FULL)
	{
		if (!iocpMain->GetInstance()->ch[recvPacket->channelNum]->userDatas.insert(make_pair(sockData->userData->userIndex, sockData->userData)).second)
		{
			cout << sockData->userData->userName << " Channel join falied" << endl;
			return;
		}
		sockData->userData->channelNum = recvPacket->channelNum;
		sockData->userData->isWhere = ROOM_SELECT;

		sendPacket->isSuccess = true;
		sendPacket->usingRoomNumSize = iocpMain->GetInstance()->ch[recvPacket->channelNum]->usingRoomNum.size();
	}
	else
	{
		sendPacket->isSuccess = false;
		sendPacket->usingRoomNumSize = 0;
	}
	cout << iocpMain->GetInstance()->ch[recvPacket->channelNum]->usingRoomNum.size() << "    " << sendPacket->usingRoomNumSize << endl;
	sendPacket->usingRoomNumSize = iocpMain->GetInstance()->ch[recvPacket->channelNum]->usingRoomNum.size();
	if (sendPacket->usingRoomNumSize != 0)
	{
		int i = 0;
		set<short>::iterator iter;
		for (iter = iocpMain->GetInstance()->ch[recvPacket->channelNum]->usingRoomNum.begin();
			iter != iocpMain->GetInstance()->ch[recvPacket->channelNum]->usingRoomNum.end(); iter++)
		{
			if (!iocpMain->GetInstance()->ch[recvPacket->channelNum]->roomDatas.find(*iter)->second->isFull) //풀방 생략
			{
				sendPacket->usingRoomNum[i] = *iter;
				i++;
			}
		}
	}
	cout << "Requset Join Channel, channel No : " << recvPacket->channelNum << endl;
	sockData->sendData = tmp_packet;
	sockData->byteSize = sizeof(AnswerJoinChannelPacket);
	PostSend(*sockData);
}

void RequestExitChannel(UserPacketData* userPacketData)
{
	SocketData* sockData = userPacketData->userSocketData;	

	sockData->userData->isWhere = CHANNEL_SELECT;
	iocpMain->GetInstance()->ch[sockData->userData->channelNum]->
		userDatas.erase(iocpMain->GetInstance()->ch[sockData->userData->channelNum]->userDatas.find(sockData->userData->userIndex));

	char tmp_packet[1024] = { 0, };
	AnswerExitChannelPacket* sendPacket = (AnswerExitChannelPacket*)tmp_packet;
	sendPacket->commandType = (short)ANSWER_EXIT_CHANNEL;
	for (int i = 0; i < MAX_CHANNEL; i++)
	{
		sendPacket->channelState[i] = (short)iocpMain->GetInstance()->ch[i]->GetChannelState();
	}
	for (int i = 0; i < MAX_CHANNEL; i++)
	{
		sendPacket->users[i] = (short)iocpMain->GetInstance()->ch[i]->userDatas.size();
	}
	sockData->sendData = tmp_packet;
	sockData->byteSize = sizeof(AnswerExitChannelPacket);
	PostSend(*sockData);
}

void RequestCreateRoom(UserPacketData* userPacketData)
{
	SocketData* sockData = userPacketData->userSocketData;	
	RequestCreateRoomPacket* recvPacket = reinterpret_cast<RequestCreateRoomPacket*>(userPacketData->userPacket);

	bool isCreate = false;
	short roomNum = iocpMain->GetInstance()->ch[sockData->userData->channelNum]->CreateRoom(*sockData->userData);
	if (roomNum != 0)
	{
		sockData->userData->roomNum = roomNum;
		sockData->userData->isWhere = ROOM_IN;
		isCreate = true;
	}
	else
		isCreate = false;

	char tmp_packet[1024] = { 0, };
	AnswerCreateRoomPacket* sendPacket = (AnswerCreateRoomPacket*)tmp_packet;
	sendPacket->commandType = ANSWER_CREATE_ROOM;
	sendPacket->isCreate = isCreate;
	sendPacket->roomNum = roomNum;
	if (isCreate)
	{
		map<int, UserData*>::iterator iter;
		for (iter = iocpMain->GetInstance()->ch[sockData->userData->channelNum]->userDatas.begin();
			iter != iocpMain->GetInstance()->ch[sockData->userData->channelNum]->userDatas.end(); iter++)
		{
			if (iter->second->userIndex == sockData->userData->userIndex || iter->second->isWhere == ROOM_IN)
				continue;
			else // 같은 채널 사용자에게 보내주기
			{
				char tmp_notify[1024] = { 0, };
				NotifyCreateRoomPacket* notifyPacket = (NotifyCreateRoomPacket*)tmp_notify;
				notifyPacket->commandType = NOTIFY_CREATE_ROOM;
				notifyPacket->roomNum = roomNum;
				iocpMain->GetInstance()->vSocketData[iter->second->userSockIndex]->sendData = tmp_notify;
				iocpMain->GetInstance()->vSocketData[iter->second->userSockIndex]->byteSize = sizeof(NotifyCreateRoomPacket);
				PostSend(*iocpMain->GetInstance()->vSocketData[iter->second->userSockIndex]);
			}
		}
	}
	cout << "Requset Create Room, isCreate : " << isCreate << " Room Num : " << roomNum << endl;
	sockData->sendData = tmp_packet;
	sockData->byteSize = sizeof(AnswerCreateRoomPacket);
	PostSend(*sockData);
}

void RequestJoinRoom(UserPacketData* userPacketData)
{
	SocketData* sockData = userPacketData->userSocketData;
	RequsetJoinRoomPacket* recvPacket = reinterpret_cast<RequsetJoinRoomPacket*>(userPacketData->userPacket);

	bool isJoin = false;
	isJoin = iocpMain->GetInstance()->ch[sockData->userData->channelNum]->JoinRoom(recvPacket->roomNum, *sockData->userData);

	char tmp_packet[1024] = { 0, };
	AnswerJoinRoomPacket* sendPacket = (AnswerJoinRoomPacket*)tmp_packet;
	sendPacket->commandType = ANSWER_JOIN_ROOM;
	sendPacket->isJoin = isJoin;
	sendPacket->roomNum = recvPacket->roomNum;
	if (isJoin)
	{
		sockData->userData->isWhere = ROOM_IN;
		sockData->userData->roomNum = recvPacket->roomNum;
		map<int, RoomData*>::iterator iter_RoomUserData =
			iocpMain->GetInstance()->ch[sockData->userData->channelNum]->roomDatas.find(sockData->userData->roomNum);
		map<int, UserData*>::iterator iter_UserData;
		for (iter_UserData = iter_RoomUserData->second->userDatas.begin(); iter_UserData != iter_RoomUserData->second->userDatas.end(); iter_UserData++)
		{
			if (iter_UserData->second->userIndex != sockData->userData->userIndex)
			{
				CopyMemory(sendPacket->otherUserName, iter_UserData->second->userName, strlen(iter_UserData->second->userName));

				char tmp_notify[1024] = { 0, };
				NotifyJoinNewUserPacket* notifyPacket = (NotifyJoinNewUserPacket*)tmp_notify;
				notifyPacket->commandType = NOTIFY_JOIN_NEW_PLAYER;
				CopyMemory(notifyPacket->newUserName, sockData->userData->userName, strlen(sockData->userData->userName));
				iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
				iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyJoinNewUserPacket);
				PostSend(*iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]);
			}
		}
	}
	sockData->sendData = tmp_packet;
	sockData->byteSize = sizeof(AnswerJoinRoomPacket);
	PostSend(*sockData);
}

void RequestChatRoom(UserPacketData* userPacketData)
{
	SocketData* sockData = userPacketData->userSocketData;

	if (sockData->userData->isWhere == ROOM_IN)
	{
		char userMsg[MAX_MSG_SIZE] = { 0, };
		char chatUserName[MAX_USER_NAME] = { 0, };
		
		RequestChatRoomPacket* recvPacket = reinterpret_cast<RequestChatRoomPacket*>(userPacketData->userPacket);
		CopyMemory(userMsg, recvPacket->userMsg, strlen(recvPacket->userMsg));

		map<int, RoomData*>::iterator iter_RoomUserData =
			iocpMain->GetInstance()->ch[sockData->userData->channelNum]->roomDatas.find(sockData->userData->roomNum);
		map<int, UserData*>::iterator iter_UserData;
		for (iter_UserData = iter_RoomUserData->second->userDatas.begin(); iter_UserData != iter_RoomUserData->second->userDatas.end(); iter_UserData++)
		{
			if (iter_UserData->second->userIndex != sockData->userData->userIndex)
			{
				char tmp_notify[1024] = { 0, };
				NotifyChatRoomPacket* notifyPacket = (NotifyChatRoomPacket*)tmp_notify;
				notifyPacket->commandType = NOTIFY_CHAT_ROOM;
				CopyMemory(notifyPacket->userName, sockData->userData->userName, strlen(sockData->userData->userName));
				CopyMemory(notifyPacket->userMsg, recvPacket->userMsg, strlen(recvPacket->userMsg));

				iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
				iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyChatRoomPacket);
				PostSend(*iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]);
			}
		}
	}
}

void RequestExitRoom(UserPacketData* userPacketData)
{
	SocketData* sockData = userPacketData->userSocketData;	
	RequestExitRoomPacket* recvPacket = reinterpret_cast<RequestExitRoomPacket*>(userPacketData->userPacket);

	map<int, RoomData*>::iterator iter_RoomUserData =
		iocpMain->GetInstance()->ch[sockData->userData->channelNum]->roomDatas.find(sockData->userData->roomNum);

	map<int, UserData*>::iterator iter_UserData;
	iter_RoomUserData->second->ExitRoom(sockData->userData->roomNum, *sockData->userData);

	if (sockData->userData->isWhere == ROOM_IN) //방에서 나왔을때
	{
		if (iocpMain->GetInstance()->ch[sockData->userData->channelNum]->DeleteRoom(sockData->userData->roomNum))
		{ //나왔는데 방이 없어졌을때 채널에 알리기

			for (iter_UserData = iocpMain->GetInstance()->ch[sockData->userData->channelNum]->userDatas.begin();
				iter_UserData != iocpMain->GetInstance()->ch[sockData->userData->channelNum]->userDatas.end(); iter_UserData++)
			{
				if (iter_UserData->second->userIndex == sockData->userData->userIndex ||
					iter_UserData->second->isWhere == ROOM_IN)
					continue;
				else
				{
					char tmp_notify[1024] = { 0, };
					NotifyDeleteRoomPacket* notifyPacket = (NotifyDeleteRoomPacket*)tmp_notify;
					notifyPacket->commandType = (short)NOTIFY_DELETE_ROOM;
					notifyPacket->deleteRoomNum = sockData->userData->roomNum;
					iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
					iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyDeleteRoomPacket);
					PostSend(*iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]);
				}
			}
		}
		else
		{ //방에 있는 사람들에게 나간것 알리기
			for (iter_UserData = iter_RoomUserData->second->userDatas.begin();
				iter_UserData != iter_RoomUserData->second->userDatas.end(); iter_UserData++)
			{
				if (iter_UserData->second->userIndex == sockData->userData->userIndex ||
					iter_UserData->second->isWhere == ROOM_IN)
					continue;
				else
				{
					char tmp_notify[1024] = { 0, };
					NotifyExitRoomPacket* notifyPacket = (NotifyExitRoomPacket*)tmp_notify;
					notifyPacket->commandType = (short)NOTIFY_EXIT_ROOM;
					CopyMemory(notifyPacket->userName, sockData->userData->userName, strlen(sockData->userData->userName));
					iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
					iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyExitRoomPacket);
					PostSend(*iocpMain->GetInstance()->vSocketData[iter_UserData->second->userSockIndex]);
				}
			}

		}
		char tmp_packet[1024] = { 0, };
		AnswerExitRoomPacket* sendPacket = (AnswerExitRoomPacket*)tmp_packet;
		sendPacket->commandType = (short)ANSWER_EXIT_ROOM;
		sendPacket->usingRoomNumSize = iocpMain->GetInstance()->ch[sockData->userData->channelNum]->usingRoomNum.size();
		int i = 0;
		set<short>::iterator iter;
		for (iter = iocpMain->GetInstance()->ch[sockData->userData->channelNum]->usingRoomNum.begin();
			iter != iocpMain->GetInstance()->ch[sockData->userData->channelNum]->usingRoomNum.end(); iter++)
		{
			if (!iocpMain->GetInstance()->ch[sockData->userData->channelNum]->roomDatas.find(*iter)->second->isFull) //풀방 생략
			{
				sendPacket->usingRoomNum[i] = *iter;
				i++;
			}
		}
		sockData->userData->isWhere = ROOM_SELECT;
		sockData->userData->roomNum = 0;
		sockData->byteSize = sizeof(AnswerExitRoomPacket);
		sockData->sendData = tmp_packet;
		PostSend(*sockData);
	}
}

