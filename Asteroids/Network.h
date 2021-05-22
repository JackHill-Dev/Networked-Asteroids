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
	int ID;
};

class ClientNetwork
{
public:
	ClientNetwork();
	void Recieve();

	std::mutex rcvMutex;
private:
	WSADATA WsaDat;
	SOCKET sock;
	SOCKADDR_IN serveraadr;
	std::queue<std::string> rcvQueue_Client;
	int serverAddrSize = sizeof(serveraadr);

	std::thread rcv_Client;
	int ID;
	
};

