#include "Network.h"

Network::Network()
{
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(27015);

	if (bind(sock, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		std::cout << "Unable to bind socket!\r\n";
		closesocket(sock);
		WSACleanup();
	}

	rcv = std::thread(&Network::Recieve, this);
	
}

void Network::Recieve()
{
	int iResult;
	const int bufferlen = 1024;
	char buffer[bufferlen];

	bool finished = false;
	std::cout << "Starting recieve thread..." << std::endl;

	while (!finished)
	{
		iResult = recvfrom(sock, buffer, bufferlen, 0, (SOCKADDR*)&clientAddr, &clientAddrSize);
		if (iResult == SOCKET_ERROR)
		{

		}
		else
		{
			std::string data;
			data = buffer;

			rcvMutex.lock();
			rcvQueue.push(data);
			rcvMutex.unlock();
		}
	
		
	}

	rcv.join();
}

void Network::CloseSockets()
{
	shutdown(sock, SD_SEND);
	closesocket(sock);
}

ClientNetwork::ClientNetwork()
{
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
	}

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
	}

	serveraadr.sin_family = AF_INET;
	serveraadr.sin_port = htons(27015);
	serveraadr.sin_addr.s_addr = inet_addr("127.0.0.1");

	rcv_Client = std::thread(&ClientNetwork::Recieve, this);
}

void ClientNetwork::Recieve()
{
	int iResult;
	const int bufferlen = 1024;
	char buffer[bufferlen];

	bool finished = false;
	std::cout << "Starting recieve thread..." << std::endl;

	while (!finished)
	{
		iResult = recvfrom(sock, buffer, bufferlen, 0, (SOCKADDR*)&serveraadr, &serverAddrSize);
		if (iResult == SOCKET_ERROR)
		{
			std::cout << "recvfrom failed with error: " << WSAGetLastError() << std::endl;
		}
		else
		{
			std::string data;
			data = buffer;


			rcvMutex.lock();
			rcvQueue_Client.push(data);
			rcvMutex.unlock();
		}


	}

	rcv_Client.join();
}
