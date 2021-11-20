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
unsigned int __stdcall AcceptThread(void* arg); // 소켓연결 스레드
unsigned int __stdcall WorkerThread(void* arg); // 소켓연결후 패킷 처리 스레드
void PostRead(SocketData& overlap);
void PostSend(SocketData& sockData);
void WorkingCommand(SocketData& overlap); // 요청한 일을 확인하고 스레드풀에 넣는 함수

// 요청한 일을 하기 위한 함수들, 스레드풀에서 진행
bool UserPacketEnqueue(SocketData& sockData);
void ResetRecvBuff(SocketData& sockData);
void RequestLogin(UserPacketData* userPacketData);
void RequestJoinChannel(UserPacketData* userPacketData);
void RequestExitChannel(UserPacketData* userPacketData);
void RequestCreateRoom(UserPacketData* userPacketData);
void RequestJoinRoom(UserPacketData* userPacketData);
void RequestChatRoom(UserPacketData* userPacketData);
void RequestExitRoom(UserPacketData* userPacketData);