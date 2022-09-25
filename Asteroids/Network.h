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
	void Send(char* buffer, const int& bufferSize);
	void SendPlayerData(const float& x, const float& y, const float& r);
	virtual void Recieve();
	void CloseSockets();

	bool AllClientsConnected();

	std::mutex rcvMutex;
	std::queue<char*> rcvQueue;
	std::thread rcv;
	int bytesRecieved;
private:
	WSADATA WsaDat;
	SOCKET sock;
	SOCKADDR_IN server;

	
	

	SOCKADDR_IN clientAddr;
	int clientAddrSize = sizeof(clientAddr);
	int ID = 1;

	int maxPlayers = 2;

};

class ClientNetwork
{
public:
	ClientNetwork(std::string& ip);
	void Recieve();
	void Send(char* buffer, const int& bufferSize);
	void SendPlayerData(const float& x, const float& y, const float& r);
	void SendConnectionRequest();
	void Disconnect();
	int& GetID() { return ID; }
	std::mutex rcvMutex_Client;
	std::queue<char*> rcvQueue_Client;
	std::thread rcv_Client;
	int bytesRecieved = 0;
private:
	WSADATA WsaDat;
	SOCKET sock;

	SOCKADDR_IN serveraadr;
	int serverAddrSize = sizeof(serveraadr);

	
	int ID = -1;



	
	
};

