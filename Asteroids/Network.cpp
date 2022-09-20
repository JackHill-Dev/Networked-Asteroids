#include "Network.h"

enum  Client_Message : unsigned char
{
	Join,
	Leave,
	Input
};

enum Server_Message : unsigned char
{
	Join_Result,
	State
};

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
			//std::string data;
			//data = buffer;

			switch (buffer[0])
			{
			case Client_Message::Join:
			{
				buffer[0] = Server_Message::Join_Result;
				buffer[1] = ++ID;
				printf("New client connected with ID: %i", ID);

				iResult = sendto(sock, buffer, bufferlen, 0, (SOCKADDR*)&clientAddr, clientAddrSize);
			}
			break;
			default: break;
			}

			//if (data == "Connection request")
			//{
			//	std::cout << "A client has connected" << std::endl;
			//	
			//	// On connection request, send the client/player their assinged ID
			//	std::string connMsg = "ID:" + std::to_string(++ID);
			//	strcpy_s(buffer, connMsg.c_str());
			//	int bytes = sendto(sock, buffer, bufferlen, 0, (SOCKADDR*)&clientAddr, clientAddrSize);
			//	if (bytes == SOCKET_ERROR)
			//	{
			//		std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
			//	}
			//}

			//if (data == "Disconnect")
			//{
			//	finished = true;
			//	std::cout << "A client disconnected";

			//}



			rcvMutex.lock();
			rcvQueue = std::queue<std::string>();
			//rcvQueue.push(data);
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

bool Network::AllClientsConnected()
{
	return ID == maxPlayers;
}

void Network::Send(const char* msg)
{
	const int bufferSize = 2048;
	char buffer[bufferSize];

	strcpy_s(buffer, msg);

	int result = sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&clientAddr, clientAddrSize);
	if (result == SOCKET_ERROR)
	{
		std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
	}
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
		else
		{
			std::string data;
			data = buffer;

			if (!data.empty() || data != " ")
			{

				if (strstr(data.c_str(), "ID"))
				{
					std::cout << "Sucessful connection, " << data << std::endl;
					size_t pos = 0;
					pos = data.find_first_of(':');
					data = data[++pos];
					ID = std::stoi(data);
					std::cout << std::to_string(ID) << std::endl;
				}

				rcvMutex_Client.lock();
				rcvQueue_Client.push(data);
				rcvMutex_Client.unlock();
			}
		}


	}

	rcv_Client.join();
}

void ClientNetwork::Send(const char* msg)
{
	const int bufferSize = 2048;
	char buffer[bufferSize];

	strcpy_s(buffer, msg);
	
	int result = sendto(sock, buffer, bufferSize, 0, (SOCKADDR*)&serveraadr, serverAddrSize);
	if (result == SOCKET_ERROR)
	{
		std::cout << "sendto failed with error: " << WSAGetLastError() << std::endl;
	}
}
