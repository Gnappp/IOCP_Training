#pragma once
#include "AllLib.h"

#include <string>
#include <vector>
#include <process.h>
#include <iostream> 
#include <string>
#include <map>
#include <set>


using namespace std;

class ChannelData;
class RoomData;
class UserData;
class ThreadPool;
class SocketData;

typedef map<int, UserData*> USER_DATA;
typedef vector<SocketData*> vecSocketData;
typedef vecSocketData::iterator iter_vSocketData;

// ä��, ��, ������, ���� �����ϴ� Ŭ����, ��𼭵� ���ٰ����� �� �ֵ��� �̱������� ����
class IocpMain
{
public:
	vector<ChannelData*> ch;
	vector<RoomData*> rm;
	USER_DATA mUserDatas;
	HANDLE hAcceptIOCP;
	HANDLE hWorkerIOCP;
	SOCKET sockListen;
	vecSocketData vSocketData; // ����(����) �����͵�
	ThreadPool* threadPool;

	~IocpMain();

	bool InitIOCP(int maxthreads, int maxsokets, int port);
	bool InitThreadPool(int maxthreads);

	IocpMain* GetInstance();
private:
	IocpMain();
	static IocpMain* inst;
};

