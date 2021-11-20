#pragma once
#include "AllLib.h"
#include "Define.h"

#include <iostream>
#include <process.h>
#include <iostream> 

using namespace std;

class SocketData;
struct stOverlap;

void CloseSocket(SocketData& overlap, bool force = false, bool listenAgain = true);
void CloseSocket(SocketData& overlap, bool force, bool listenAgain);
void InitOVERLAP(SocketData& overlap);
unsigned int __stdcall AcceptThread(void* arg); // ���Ͽ��� ������
unsigned int __stdcall WorkerThread(void* arg); // ���Ͽ����� ��Ŷ ó�� ������
void PostRead(SocketData& overlap);
void PostSend(SocketData& sockData);
void WorkingCommand(SocketData& overlap); // ��û�� ���� Ȯ���ϰ� ������Ǯ�� �ִ� �Լ�

// ��û�� ���� �ϱ� ���� �Լ���, ������Ǯ���� ����
bool UserPacketEnqueue(SocketData& sockData);
void ResetRecvBuff(SocketData& sockData);
void RequestLogin(UserPacketData* userPacketData);
void RequestJoinChannel(UserPacketData* userPacketData);
void RequestExitChannel(UserPacketData* userPacketData);
void RequestCreateRoom(UserPacketData* userPacketData);
void RequestJoinRoom(UserPacketData* userPacketData);
void RequestChatRoom(UserPacketData* userPacketData);
void RequestExitRoom(UserPacketData* userPacketData);