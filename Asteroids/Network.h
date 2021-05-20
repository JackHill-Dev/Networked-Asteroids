#pragma once
#include <winsock2.h>
#include <WS2tcpip.h>
#include <queue>
#include <mutex>
#include <string>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")	// Use this library whilst linking - contains the Winsock2 implementation.

class Network
{
public:
	Network();
	void Send(SOCKET* sock, char* msg, int msgSize);
	virtual void Recieve();
	void CloseSockets();

	std::mutex rcvMutex;
	
	
private:
	WSADATA WsaDat;
	SOCKET sock;
	SOCKADDR_IN server;

	std::thread rcv;
	std::queue<std::string> rcvQueue;

	SOCKADDR_IN clientAddr;
	int clientAddrSize = sizeof(clientAddr);
};

class ClientNetWork : public Network
{
public:
	virtual void Recieve() override;
private:
	WSADATA WsaDat;
	SOCKET sock;
	SOCKADDR_IN serveraadr;
	int serverAddrSize = sizeof(serveraadr);
};

