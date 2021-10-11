#pragma once
#include <iostream>
#include <process.h>
#include <iostream> 
#include <WinSock2.h> 

using namespace std;

class SocketData;
struct stOverlap;

void CloseSocket(SocketData& overlap, bool force = false, bool listenAgain = true);
void CloseSocket(SocketData& overlap, bool force, bool listenAgain);
void InitOVERLAP(SocketData& overlap);
unsigned int __stdcall AcceptThread(void* arg);
unsigned int __stdcall WorkerThread(void* arg);
void PostRead(SocketData& overlap);
void PostSend(SocketData& sockData);
void WorkingCommand(SocketData& overlap);
//bool DoEcho(SocketData& overlap);

bool UserPacketEnqueue(SocketData& sockData);
void UserPacketDequeue(SocketData& sockData, int packetSize);
void ResetRecvBuff(SocketData& sockData);
void RequestLogin(SocketData& sockData, int packetSize);
void RequestJoinChannel(SocketData& sockData, int packetSize);
void RequestExitChannel(SocketData& sockData, int packetSize);
void RequestCreateRoom(SocketData& sockData, int packetSize);
void RequestJoinRoom(SocketData& sockData, int packetSize);
void RequestChatRoom(SocketData& sockData, int packetSize);
void RequestExitRoom(SocketData& sockData, int packetSize);