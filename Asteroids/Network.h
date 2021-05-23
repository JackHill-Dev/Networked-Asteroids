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
	void Send(const char* msg);
	virtual void Recieve();
	void CloseSockets();

	bool AllClientsConnected();

	std::mutex rcvMutex;
	std::queue<std::string> rcvQueue;
	
private:
	WSADATA WsaDat;
	SOCKET sock;
	SOCKADDR_IN server;

	std::thread rcv;
	

	SOCKADDR_IN clientAddr;
	int clientAddrSize = sizeof(clientAddr);
	int ID = 1;

	int maxPlayers = 2;
};

class ClientNetwork
{
public:
	ClientNetwork();
	void Recieve();
	void Send(const char* msg);

	std::mutex rcvMutex_Client;
	std::queue<std::string> rcvQueue_Client;
private:
	WSADATA WsaDat;
	SOCKET sock;

	SOCKADDR_IN serveraadr;
	int serverAddrSize = sizeof(serveraadr);

	std::thread rcv_Client;
	int ID = -1;

	
	
};

