#pragma once
#include <string>
#include <vector>
#include <process.h>
#include <iostream> 
#include <WinSock2.h> 
#include <string>
#include <map>
#include <set>

#pragma comment(lib, "ws2_32.lib") 
#pragma comment(lib, "mswsock.lib")

using namespace std;

class ChannelData;
class RoomData;
class UserData;
class ThreadPool;
class SockData;

typedef map<int, UserData*> USER_DATA;
typedef vector<SocketData*> vecSocketData;
typedef vecSocketData::iterator iter_vSocketData;

class IocpMain
{
public:
	vector<ChannelData* > ch;
	set<RoomData* > rm;
	USER_DATA mUserDatas;
	HANDLE hAcceptIOCP;
	HANDLE hWorkerIOCP;
	SOCKET sockListen;
	vecSocketData vSocketData; //소켓 데이터들
	ThreadPool* threadPool;

	IocpMain();
	~IocpMain();

	bool InitIOCP(int maxthreads, int maxsokets, int port);
	bool InitThreadPool(int maxthreads);
};
