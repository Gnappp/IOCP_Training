#pragma once
#include <iostream>
#include <process.h>
#include <iostream> 
#include <WinSock2.h> 

using namespace std;

class SocketData;
class IocpMain;
struct stOverlap;

extern IocpMain* iocpMain;

void CloseSocket(SocketData& overlap, bool force = false, bool listenAgain = true);
void CloseSocket(SocketData& overlap, bool force, bool listenAgain);
void InitOVERLAP(SocketData& overlap);
unsigned int __stdcall AcceptThread(void* arg);
unsigned int __stdcall WorkerThread(void* arg);
void PostRead(SocketData& overlap);
void PostSend(SocketData& sockData);
void WorkingCommand(SocketData& overlap);
bool DoEcho(SocketData& overlap);