#pragma once
#include <iostream> 
#include <WinSock2.h> 
#include "IOCP_main.h"
#include "Packets.h"
#include "Define.h"

// 초기화할때 AcceptEx를 이용하여 리슨될때 생성한 소켓으로 갈 수 있게 해준다.
// AcceptEx(리슨소켓, 만들어진넣을소켓, 수신된 데이터를 받을 버퍼[char []],	받을 데이터크기[int], 소켓 ip크기[int], 소켓 ip크기, 받은데이터 크기[DWORD], overlap 구조체
// GetAcceptExSockaddrs(overlap의 버퍼크기,	받을 데이터크기, 소켓 ip크기, 소켓 ip크기, 로컬ip받을 sockaddr_in, 로컬 ip 크기[int], 리모트 ip받을 sockaddr_in, 리모트 ip 크기[int])

void PostSend(SocketData& sockData);

SocketData::SocketData(SOCKET& listenSock, int iSock_num)
{
	memset(this, NULL, sizeof(*this));
	int zero = 0;
	isConnected = false;
	ZeroMemory(bufRecvData, BUF_SIZE);
	ZeroMemory(&recvBuf, sizeof(WSABUF));
	sendData = { 0, };
	if (userData != NULL)
		free(userData);
	userData = new UserData(iSock_num);
	int userSockIndex = iSock_num;

	bufEnd = &bufRecvData[0];
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
		cout << "Fail create sock" << endl;
	setsockopt(sock, SOL_SOCKET, SO_SNDBUF, (char*)&zero, sizeof(zero));
	setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*)&zero, sizeof(zero));

	sendOverLap.state = SEND;

	recvOverLap.state = RECV;
	recvOverLap.sockData = this;
	AcceptEx(listenSock, sock, bufEnd, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &byteSize, (OVERLAPPED*)&recvOverLap);
}

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
		result = GetQueuedCompletionStatus(hAcceptIOCP, &nByteSize, (PULONG_PTR)&key, (OVERLAPPED**)&overlap, INFINITE);
		cout << "Accept Connect!" << endl;

		if (!result || overlap == NULL)
		{
			if (overlap != NULL)
			{
				cout << "Accept socket close" << endl;
				CloseSocket(*(overlap->sockData), true, sockListen == INVALID_SOCKET ? false : true);
			}
		}
		else
		{
			if (!overlap->sockData->isConnected)
			{
				GetAcceptExSockaddrs(&overlap->sockData->bufRecvData[0], 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, (sockaddr**)&local, &localLen, (sockaddr**)&remote, &remoteLen);
				memcpy(&overlap->sockData->mLocal, local, sizeof(sockaddr_in));
				memcpy(&overlap->sockData->mRemote, remote, sizeof(sockaddr_in));
				setsockopt(overlap->sockData->sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&sockListen, sizeof(sockListen));
				overlap->sockData->isConnected = true;

				CreateIoCompletionPort((HANDLE)overlap->sockData->sock, hWorkerIOCP, (DWORD)overlap->sockData, 0);
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
		result = GetQueuedCompletionStatus(hWorkerIOCP, &nByteSize, (PULONG_PTR)&key, (OVERLAPPED**)&overlap, INFINITE);

		if (!result || overlap == NULL)
		{
			if (overlap != NULL)
			{
				cout << "Worker socket close" << endl;
				CloseSocket(*(overlap->sockData), true, sockListen == INVALID_SOCKET ? false : true);
			}
		}
		else
		{
			if (overlap->state == RECV)
			{
				overlap->sockData->byteSize = nByteSize;
				if (nByteSize != 0)
					WorkingCommand(*(overlap->sockData));
			}
		}
	}
}

void WorkingCommand(SocketData& sockData)
{
	int localLen, remoteLen;
	sockaddr_in* local = 0, * remote = 0;
	short commandType = 0;


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
		CopyMemory(&commandType, sockData.bufEnd, sizeof(short));
		sockData.bufEnd += sockData.byteSize;
		
		cout << sockData.userSockIndex <<"  Command Type : " << commandType << endl;
		switch (commandType)
		{
		case REQUEST_LOGIN:
		{
			RequestLoginPacket* packet = reinterpret_cast<RequestLoginPacket*>(sockData.bufRecvData);
			strcpy_s(sockData.userData->userName, MAX_USER_NAME, packet->userName);
			cout << sockData.userSockIndex << " : " << packet->userName << endl;

			sockData.userData->userIndex = rand();
			if (mUserDatas.find(sockData.userData->userIndex) != mUserDatas.end())
			{
				sockData.userData->userIndex = rand();
				while (1)
				{
					if (mUserDatas.find(sockData.userData->userIndex) != mUserDatas.end())
						sockData.userData->userIndex = rand();
					else
						break;
				}
			}
			mUserDatas.insert(make_pair(sockData.userData->userIndex, *sockData.userData));
			sockData.userData->isWhere = CHANNEL_SELECT;

			char tmp_packet[1024] = { 0, };
			AnswerLoginPacket* sendPacket = (AnswerLoginPacket*)tmp_packet;
			sendPacket->commandType = (short)ANSWER_LOGIN;
			for (int i = 0; i < MAX_CHANNEL; i++)
			{
				sendPacket->channelState[i] = (short)ch[i].GetChannelState();
			}
			for (int i = 0; i < MAX_CHANNEL; i++)
			{
				sendPacket->users[i] = (short)ch[i].userDatas.size();
			}
			CopyMemory(sendPacket->userName, sockData.userData->userName, strlen(sockData.userData->userName));
			sockData.sendData = tmp_packet;
			sockData.byteSize = sizeof(AnswerLoginPacket);
			PostSend(sockData);
			break;
		}
		case REQUEST_JOIN_CHANNEL:
		{
			RequestJoinChannelPacket* recvPacket = reinterpret_cast<RequestJoinChannelPacket*>(sockData.bufRecvData);

			char tmp_packet[1024] = { 0, };
			AnswerJoinChannelPacket* sendPacket = (AnswerJoinChannelPacket*)tmp_packet;
			sendPacket->commandType = (short)ANSWER_JOIN_CHANNEL;
			cout << "Channel No "<<(short)recvPacket->channelNum << endl;

			if (ch[recvPacket->channelNum].GetChannelState() != CHANNEL_USER_FULL)
			{
				if (!ch[recvPacket->channelNum].userDatas.insert(make_pair(sockData.userData->userIndex, sockData.userData)).second)
				{
					cout << sockData.userData->userName << " Channel join falied" << endl;
					break;
				}
				sockData.userData->channelNum = recvPacket->channelNum;
				sockData.userData->isWhere = ROOM_SELECT;

				sendPacket->isSuccess = true;
				sendPacket->usingRoomNumSize = ch[recvPacket->channelNum].usingRoomNum.size();
			}
			else
			{
				sendPacket->isSuccess = false;
				sendPacket->usingRoomNumSize = 0;
			}
			sendPacket->usingRoomNumSize = ch[recvPacket->channelNum].usingRoomNum.size();
			int i = 0;
			set<short>::iterator iter;
			for (iter = ch[recvPacket->channelNum].usingRoomNum.begin(); iter != ch[recvPacket->channelNum].usingRoomNum.end(); iter++)
			{
				if (!ch[recvPacket->channelNum].roomDatas.find(*iter)->second->isFull) //풀방 생략
				{
					sendPacket->usingRoomNum[i] = *iter;
					i++;
				}
			}
			cout << "Requset Join Channel, channel No : " << recvPacket->channelNum <<  endl;
			sockData.sendData = tmp_packet;
			sockData.byteSize = sizeof(AnswerJoinChannelPacket);
			PostSend(sockData);
			break;
		}
		case REQUEST_EXIT_CHANNEL:
		{
			sockData.userData->isWhere = CHANNEL_SELECT;
			ch[sockData.userData->channelNum].userDatas.erase(ch[sockData.userData->channelNum].userDatas.find(sockData.userData->userIndex));

			char tmp_packet[1024] = { 0, };
			AnswerExitChannelPacket* sendPacket = (AnswerExitChannelPacket*)tmp_packet;
			sendPacket->commandType = (short)ANSWER_EXIT_CHANNEL;
			for (int i = 0; i < MAX_CHANNEL; i++)
			{
				sendPacket->channelState[i] = (short)ch[i].GetChannelState();
			}
			for (int i = 0; i < MAX_CHANNEL; i++)
			{
				sendPacket->users[i] = (short)ch[i].userDatas.size();
			}
			sockData.sendData = tmp_packet;
			sockData.byteSize = sizeof(AnswerExitChannelPacket);
			PostSend(sockData);
			break;
		}
		case REQUEST_CREATE_ROOM:
		{
			RequestCreateRoomPacket* recvPacket = reinterpret_cast<RequestCreateRoomPacket*>(sockData.bufRecvData);
			bool isCreate = false;
			short roomNum = ch[sockData.userData->channelNum].CreateRoom(*sockData.userData);
			if (roomNum != 0)
			{
				sockData.userData->roomNum = roomNum;
				sockData.userData->isWhere = ROOM_IN;
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
				for (iter = ch[sockData.userData->channelNum].userDatas.begin();
					iter != ch[sockData.userData->channelNum].userDatas.end(); iter++)
				{
					if (iter->second->userIndex == sockData.userData->userIndex || iter->second->isWhere == ROOM_IN)
						continue;
					else // 같은 채널 사용자에게 보내주기
					{
						char tmp_notify[1024] = { 0, };
						NotifyCreateRoomPacket* notifyPacket = (NotifyCreateRoomPacket*)tmp_notify;
						notifyPacket->commandType = NOTIFY_CREATE_ROOM;
						notifyPacket->roomNum = roomNum;
						vSocketData[iter->second->userSockIndex]->sendData = tmp_notify;
						vSocketData[iter->second->userSockIndex]->byteSize = sizeof(NotifyCreateRoomPacket);
						PostSend(*vSocketData[iter->second->userSockIndex]);
					}
				}
			}
			cout << "Requset Create Room, isCreate : " << isCreate  << " Room Num : "<< roomNum<< endl;
			sockData.sendData = tmp_packet;
			sockData.byteSize = sizeof(AnswerCreateRoomPacket);
			PostSend(sockData);
			break;
		}
		case REQUEST_JOIN_ROOM:
		{
			RequsetJoinRoomPacket* recvPacket = reinterpret_cast<RequsetJoinRoomPacket*>(sockData.bufRecvData);
			bool isJoin = false;
			isJoin = ch[sockData.userData->channelNum].JoinRoom(recvPacket->roomNum, *sockData.userData);

			char tmp_packet[1024] = { 0, };
			AnswerJoinRoomPacket* sendPacket = (AnswerJoinRoomPacket*)tmp_packet;
			sendPacket->commandType = ANSWER_JOIN_ROOM;
			sendPacket->isJoin = isJoin;
			sendPacket->roomNum = recvPacket->roomNum;
			if (isJoin)
			{
				sockData.userData->isWhere = ROOM_IN;
				sockData.userData->roomNum = recvPacket->roomNum;
				map<int, RoomData*>::iterator iter_RoomUserData = ch[sockData.userData->channelNum].roomDatas.find(sockData.userData->roomNum);
				map<int, UserData*>::iterator iter_UserData;
				for (iter_UserData = iter_RoomUserData->second->userDatas.begin(); iter_UserData != iter_RoomUserData->second->userDatas.end(); iter_UserData++)
				{
					if (iter_UserData->second->userIndex != sockData.userData->userIndex)
					{
						CopyMemory(sendPacket->otherUserName, iter_UserData->second->userName, strlen(iter_UserData->second->userName));

						char tmp_notify[1024] = { 0, };
						NotifyJoinNewUserPacket* notifyPacket = (NotifyJoinNewUserPacket*)tmp_notify;
						notifyPacket->commandType = NOTIFY_JOIN_NEW_PLAYER;
						CopyMemory(notifyPacket->newUserName, sockData.userData->userName, strlen(sockData.userData->userName));
						vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
						vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyJoinNewUserPacket);
						PostSend(*vSocketData[iter_UserData->second->userSockIndex]);
					}
				}
			}
			sockData.sendData = tmp_packet;
			sockData.byteSize = sizeof(AnswerJoinRoomPacket);
			PostSend(sockData);
			break;
		}
		case REQUEST_CHAT_ROOM:
		{
			if (sockData.userData->isWhere == ROOM_IN)
			{
				char userMsg[MAX_MSG_SIZE] = { 0, };
				char chatUserName[MAX_USER_NAME] = { 0, };
				RequestChatRoomPacket* recvPacket = reinterpret_cast<RequestChatRoomPacket*>(sockData.bufRecvData);
				CopyMemory(userMsg, recvPacket->userMsg, strlen(recvPacket->userMsg));

				map<int, RoomData*>::iterator iter_RoomUserData = ch[sockData.userData->channelNum].roomDatas.find(sockData.userData->roomNum);
				map<int, UserData*>::iterator iter_UserData;
				for (iter_UserData = iter_RoomUserData->second->userDatas.begin(); iter_UserData != iter_RoomUserData->second->userDatas.end(); iter_UserData++)
				{
					if (iter_UserData->second->userIndex != sockData.userData->userIndex)
					{
						char tmp_notify[1024] = { 0, };
						NotifyChatRoomPacket* notifyPacket = (NotifyChatRoomPacket*)tmp_notify;
						notifyPacket->commandType = NOTIFY_CHAT_ROOM;
						CopyMemory(notifyPacket->userName, sockData.userData->userName, strlen(sockData.userData->userName));
						CopyMemory(notifyPacket->userMsg, recvPacket->userMsg, strlen(recvPacket->userMsg));

						vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
						vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyChatRoomPacket);
						PostSend(*vSocketData[iter_UserData->second->userSockIndex]);
					}
				}
			}
			break;
		}
		case REQUEST_EXIT_ROOM:
		{
			RequestExitRoomPacket* recvPacket = reinterpret_cast<RequestExitRoomPacket*>(sockData.bufRecvData);
			map<int, RoomData*>::iterator iter_RoomUserData = ch[sockData.userData->channelNum].roomDatas.find(sockData.userData->roomNum);
			map<int, UserData*>::iterator iter_UserData;
			iter_RoomUserData->second->ExitRoom(sockData.userData->roomNum, *sockData.userData);

			if (sockData.userData->isWhere == ROOM_IN) //방에서 나왔을때
			{
				if (ch[sockData.userData->channelNum].DeleteRoom(sockData.userData->roomNum))
				{ //나왔는데 방이 없어졌을때 채널에 알리기
					
					for (iter_UserData = ch[sockData.userData->channelNum].userDatas.begin();
						iter_UserData != ch[sockData.userData->channelNum].userDatas.end(); iter_UserData++)
					{
						if (iter_UserData->second->userIndex == sockData.userData->userIndex ||
							iter_UserData->second->isWhere == ROOM_IN)
							continue;
						else
						{
							char tmp_notify[1024] = { 0, };
							NotifyDeleteRoomPacket* notifyPacket = (NotifyDeleteRoomPacket*)tmp_notify;
							notifyPacket->commandType = (short)NOTIFY_DELETE_ROOM;
							notifyPacket->deleteRoomNum = sockData.userData->roomNum;
							vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
							vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyDeleteRoomPacket);
							PostSend(*vSocketData[iter_UserData->second->userSockIndex]);
						}
					}
				}
				else
				{ //방에 있는 사람들에게 나간것 알리기
					for (iter_UserData = iter_RoomUserData->second->userDatas.begin(); 
						iter_UserData != iter_RoomUserData->second->userDatas.end(); iter_UserData++)
					{ 
						if (iter_UserData->second->userIndex == sockData.userData->userIndex ||
							iter_UserData->second->isWhere == ROOM_IN)
							continue;
						else
						{
							char tmp_notify[1024] = { 0, };
							NotifyExitRoomPacket* notifyPacket = (NotifyExitRoomPacket*)tmp_notify;
							notifyPacket->commandType = (short)NOTIFY_EXIT_ROOM;
							CopyMemory(notifyPacket->userName, sockData.userData->userName, strlen(sockData.userData->userName));
							vSocketData[iter_UserData->second->userSockIndex]->sendData = tmp_notify;
							vSocketData[iter_UserData->second->userSockIndex]->byteSize = sizeof(NotifyExitRoomPacket);
							PostSend(*vSocketData[iter_UserData->second->userSockIndex]);
						}
					}
					
				}
				char tmp_packet[1024] = { 0, };
				AnswerExitRoomPacket* sendPacket = (AnswerExitRoomPacket*)tmp_packet;
				sendPacket->commandType = (short)ANSWER_EXIT_ROOM;
				sendPacket->usingRoomNumSize = ch[sockData.userData->channelNum].usingRoomNum.size();
				int i = 0;
				set<short>::iterator iter;
				for (iter = ch[sockData.userData->channelNum].usingRoomNum.begin(); iter != ch[sockData.userData->channelNum].usingRoomNum.end(); iter++)
				{
					if (!ch[sockData.userData->channelNum].roomDatas.find(*iter)->second->isFull) //풀방 생략
					{
						sendPacket->usingRoomNum[i] = *iter;
						i++;
					}
				}
				sockData.userData->isWhere = ROOM_SELECT;
				sockData.userData->roomNum = 0;
				sockData.byteSize = sizeof(AnswerExitRoomPacket);
				sockData.sendData = tmp_packet;
				PostSend(sockData);
			}
		}
		default:
			cout << (short)(sockData.bufRecvData[0] + sockData.bufRecvData[1]) << endl;
			break;
		}
		memset(sockData.bufRecvData, '\0', sizeof(sockData.bufRecvData));
		sockData.byteSize = 0;
		sockData.bufEnd = &sockData.bufRecvData[0];
		PostRead(sockData);
	}
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
	//cout << "Postsend Success" << endl;
}

void PostRead(SocketData& sockData)
{
	BOOL result;

	if (sockData.bufEnd == &sockData.bufRecvData[BUF_SIZE])
		sockData.bufEnd = &sockData.bufRecvData[0];

	sockData.byteSize = &sockData.bufRecvData[BUF_SIZE] - sockData.bufEnd;
	sockData.recvBuf.len = sockData.byteSize;
	sockData.recvBuf.buf = sockData.bufEnd;
	//cout << &sockData.bufRecvData[BUF_SIZE] - sockData.bufEnd << "  " << sockData.byteSize << endl;
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

bool DoEcho(SocketData& sockData)
{
	char buff[BUF_SIZE];
	memcpy(buff, sockData.bufRecvData, sockData.byteSize);
	buff[sockData.byteSize] = '\0';
	cout << sockData.userSockIndex << " : " << sockData.bufRecvData << endl;
	memset(sockData.bufRecvData, '\0', sizeof(sockData.bufRecvData));
	sockData.byteSize = 0;
	sockData.bufEnd = &sockData.bufRecvData[0];
	//Echo인 이유
	iter_vSocketData iter;
	for (iter = vSocketData.begin(); iter != vSocketData.end(); iter++)
	{
		if ((*iter)->recvOverLap.sockData->isConnected)
		{
			(*iter)->sendData = buff;
			PostSend(*(*iter));
		}
	}
	return true;
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

int main()
{
	int maxthreads = 4, port = 1500, maxsockets = 4;
	int zero = 0;
	vector<SocketData> vOverlap;
	iter_vSocketData iterOverlap;
	HANDLE h;
	unsigned int dummy;

	hAcceptIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	hWorkerIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (hAcceptIOCP == NULL || hWorkerIOCP == NULL)
		return 0;

	//Create Thread
	for (int i = 0; i < maxthreads; i++)
	{
		h = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, NULL, 0, &dummy);
		if (h == NULL) return 0;
		CloseHandle(h);
	}

	for (int i = 0; i < maxthreads; i++)
	{
		h = (HANDLE)_beginthreadex(NULL, 0, AcceptThread, NULL, 0, &dummy);
		if (h == NULL) return 0;
		CloseHandle(h);
	}

	WSADATA wd = { 0, };
	WSAStartup(MAKEWORD(2, 2), &wd);

	//Create sockListen
	SOCKADDR_IN addr;
	//sockListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons((short)port);
	if (bind(sockListen, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		return 0;
	if (listen(sockListen, 200) == SOCKET_ERROR)
		return 0;

	//Create accept Overlap structur
	vSocketData.clear();
	for (int i = 0; i < maxsockets; i++)
	{
		SocketData* sockData = new SocketData(sockListen, i);
		//InitOVERLAP(*pOverlap);
		vSocketData.push_back(sockData);
	}

	CreateIoCompletionPort((HANDLE)sockListen, hAcceptIOCP, 0, 0);

	for (int i = 0; i < MAX_CHANNEL; i++)
	{
		ChannelData chData(i);
		ch.push_back(chData);
	}

	while (TRUE) {};
	for (iterOverlap = vSocketData.begin(); iterOverlap != vSocketData.end(); iterOverlap++)
	{
		closesocket((*iterOverlap)->sock);
		delete (&iterOverlap);
	}
	CloseHandle(hAcceptIOCP); // shut down winsock WSACleanup(); return 0;
	CloseHandle(hWorkerIOCP);
}

