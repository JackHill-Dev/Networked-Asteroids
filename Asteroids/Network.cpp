#include "Network.h"
#include "Messages.h"


Network::Network()
{
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
	}

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = htonl( INADDR_ANY);
	server.sin_port = htons(27015);

	if (bind(sock, (SOCKADDR*)&server, sizeof(server)) == SOCKET_ERROR)
	{
		std::cout << "Unable to bind socket, error: " << WSAGetLastError()  << std::endl;
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
			std::cout << "recvfrom faliled with error: " << WSAGetLastError() << std::endl;
		}
		else
		{
			switch (buffer[0])
			{
			case Client_Message::Join:
			{
				buffer[0] = Server_Message::Join_Result;
				ID++;
				memcpy(&buffer[1], &ID, sizeof(ID));
				printf("New client connected with ID: %i", ID);

				iResult = sendto(sock, buffer, bufferlen, 0, (SOCKADDR*)&clientAddr, clientAddrSize);
			}
			break;
			case Client_Message::Leave:
			{
				// Reduce amount of clients/players by 1
				ID--;
				// End this clients thread on the server
				printf("Client disconnected! Current players left: %i", ID);
				
			}
			break;
			case Client_Message::Input:
			{
				rcvMutex.lock();
				rcvQueue = std::queue<char*>();
				rcvQueue.push(buffer);
				rcvMutex.unlock();
			}
			break;
			default: break;
			}

			buffer[0] = Server_Message::State;
			int32_t bytesWritten = 1;

		}

	}

	rcv.join();
}

void Network::CloseSockets()
{
	shutdown(sock, SD_SEND);
	closesocket(sock);
}

bool Network::AllClientsConnected()
{
	return ID == maxPlayers;
}

void Network::Send(char* buffer, const int& bufferSize)
{
	int result = sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&clientAddr, clientAddrSize);
	if (result == SOCKET_ERROR)
	{
		std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
	}
}

void Network::SendPlayerData(const float& x,const float& y, const float& r)
{
	const int bufferSize = 1024;
	char buffer[bufferSize];
	uint32_t bytesWritten = 0;

	buffer[0] = Server_Message::State;
	bytesWritten = 1;

	memcpy(&buffer[bytesWritten], &x, sizeof(float));
	bytesWritten += sizeof(float);

	memcpy(&buffer[bytesWritten], &y, sizeof(float));
	bytesWritten += sizeof(float);

	memcpy(&buffer[bytesWritten], &r, sizeof(float));
	bytesWritten += sizeof(float);

	sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&clientAddr, clientAddrSize);
}


ClientNetwork::ClientNetwork(std::string& ip)
{
	const char* serverIP = ip.c_str();

	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
	}

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
	}

	serveraadr.sin_family = AF_INET;
	serveraadr.sin_port = htons(27015);
	inet_pton(AF_INET, serverIP, &serveraadr.sin_addr.s_addr);


	rcv_Client = std::thread(&ClientNetwork::Recieve, this);
}

void ClientNetwork::Recieve()
{
	int iResult;
	const int bufferlen = 2048;
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

		switch (buffer[0])
		{
		case Server_Message::Join_Result:
		{
			// Get and set the allocated ID from the server
			memcpy(&ID, &buffer[1], sizeof(ID));
			printf("Connection to server successful! Allocated ID is: %i", ID);
		}
		break;
		case Server_Message::State:
		{
			rcvMutex_Client.lock();
			rcvQueue_Client.push(buffer);
			rcvMutex_Client.unlock();
		}
		break;
		}

	}

	rcv_Client.join();
}

void ClientNetwork::Send(char* buffer, const int& bufferSize)
{
	
	int result = sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&serveraadr, serverAddrSize);
	if (result == SOCKET_ERROR)
	{
		std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
	}
}

void ClientNetwork::SendPlayerData(const float& x, const float& y, const float& r)
{
	const int bufferSize = 1024;
	char buffer[bufferSize];
	uint32_t bytesWritten = 0;

	buffer[0] = Client_Message::Input;
	bytesWritten = 1;

	memcpy(&buffer[bytesWritten], &x, sizeof(float));
	bytesWritten += sizeof(float);

	memcpy(&buffer[bytesWritten], &y, sizeof(float));
	bytesWritten += sizeof(float);

	memcpy(&buffer[bytesWritten], &r, sizeof(float));
	bytesWritten += sizeof(float);

	sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&serveraadr, serverAddrSize);
}

void ClientNetwork::SendConnectionRequest()
{
	const int bufferSize = 1024;
	char buffer[bufferSize];

	buffer[0] = Client_Message::Join;

	int result = sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&serveraadr, serverAddrSize);
	if (result == SOCKET_ERROR)
	{
		std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
	}
}

void ClientNetwork::Disconnect()
{
	const int bufferSize = 1024;
	char buffer[bufferSize];

	buffer[0] = Client_Message::Leave;

	int result = sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&serveraadr, serverAddrSize);
	if (result == SOCKET_ERROR)
	{
		std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
	}
}
