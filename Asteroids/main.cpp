#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <process.h>
#include <string>
#include <thread>
#include "Game.h"

#pragma comment(lib, "ws2_32.lib")	// Use this library whilst linking - contains the Winsock2 implementation.
#define PORT 27015;
// Host client queues and critical sections
CRITICAL_SECTION CriticalSection_Recieve;
CRITICAL_SECTION CriticalSection_Send;

std::queue<std::string> game_data_recieve_queue;
std::queue<std::string> game_data_send_queue;
//----

// Client queues and critical sections
CRITICAL_SECTION CS_Client_Rcv;
CRITICAL_SECTION CS_Client_Snd;

std::queue<std::string> game_data_client_rcv_queue;
std::queue<std::string> game_data_client_snd_queue;
//----

void ReceiveFunction(SOCKET* sock, bool* running)
{
	int iResult, iSendResult;
	SOCKET ClientSocket = *sock;

	const int buffer_size = 1024;
	char buffer[buffer_size];

	do 
	{
		iResult = recv(ClientSocket, buffer, buffer_size, 0);
		if (iResult > 0) {
			printf("Bytes received: %d\n", iResult);

			// Echo the buffer back to the sender
			iSendResult = send(ClientSocket, buffer, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			
		}
	} while (iResult > 0);

	//std::cout << "Starting Winsock on thread." << std::endl;
	////bool finished = false;

	//// Repeat 
	//while (running)
	//{
	//	const int buffer_size = 1024;// sizeof(sf::Vector2f);
	//	char buffer[buffer_size];	// Space for the data.

	//	struct sockaddr_in client_address;	// Placeholder for client address information - 'recvfrom' will fill this in and return the size of this data in client_address_size.
	//	int client_address_size = sizeof(client_address);

	//	std::cout << "Waiting for data from client..." << std::endl;
	//	SecureZeroMemory(buffer, buffer_size);
	//	int bytes_received = recvfrom(*sock, buffer, buffer_size, 0, (SOCKADDR*)&client_address, &client_address_size);

	//	if (bytes_received == SOCKET_ERROR)
	//	{	// If there is an error, deal with it here...
	//		std::cout << "'recvfrom' failed with error " << WSAGetLastError();
	//		std::cout << std::endl;
	//	}
	//	else
	//	{
	//		// No error, so put the data on the queue for the main thread.
	//		std::string t = buffer;	// t contains the string sent here from the current client.

	//		if (t == "quit") *running = true;	// Message from the main thread to terminate this thread.

	//		EnterCriticalSection(&CriticalSection_Recieve);
	//		game_data_recieve_queue.push(t);
	//		LeaveCriticalSection(&CriticalSection_Recieve);

	//	}
	//}
	
}

void ClientRcv(SOCKET* sock, bool* appRunning)
{
	SOCKET rcvSocket = *sock;

	const int buffer_size = 1024;
	char buffer[buffer_size];

	ZeroMemory(buffer, buffer_size);

	int iResult;
	std::string data;

	

	do 
	{
		std::cout << "Recieve thread running" << std::endl;
		iResult = recv(rcvSocket, buffer, buffer_size, 0);
		if (iResult > 0)
		{
			data = buffer;
			std::cout << data << std::endl;
		}
		else if (iResult == 0)
		{
			std::cout << "Connection closed!" << std::endl;
		}
		else
			std::cout << "Rcv failed with error: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);

	//struct sockaddr_in server_address;
	//int server_address_size = (int)sizeof(server_address);
	//short port = PORT;	// Port number - can change this, but needs to be the same on both client and server.
	//const char* server_ip_address = "127.0.0.1";	// The local host - change this for proper IP address of server if not on the local machine.
	//server_address.sin_family = AF_INET;
	//server_address.sin_port = htons(port);
	//server_address.sin_addr.s_addr = inet_addr(server_ip_address);

	//while (appRunning)
	//{
	//	std::cout << "Recieve thread running" << std::endl;

	//	int bytes_received = recvfrom(*sock, buffer, buffer_size, 0, (SOCKADDR*)&server_address, &server_address_size);
	//	if (bytes_received == SOCKET_ERROR)
	//	{	// If there is an error, deal with it here...
	//		std::cout << "recvfrom failed with error " << WSAGetLastError();
	//	}
	//	else
	//	{
	//		EnterCriticalSection(&CS_Client_Rcv);
	//		std::string vel = buffer;
	//		game_data_client_rcv_queue.push(vel);
	//		LeaveCriticalSection(&CS_Client_Rcv);
	//	}
	//	
	//}

	std::cout << "Out of loop" << std::endl;
}

void SendFunction(SOCKET* sock, bool* running)
{
	const int buffer_size = 1024;
	char buffer[buffer_size];
	SecureZeroMemory(&buffer, buffer_size);

	struct sockaddr_in server_address;
	int server_address_size = (int)sizeof(server_address);
	short port = PORT;	// Port number - can change this, but needs to be the same on both client and server.
	const char* server_ip_address = "localhost";	// The local host - change this for proper IP address of server if not on the local machine.
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr(server_ip_address);

	
	while (running)
	{
		std::string gData = "";
		
		if (game_data_send_queue.size() > 0)
		{
			EnterCriticalSection(&CriticalSection_Send);
			gData = game_data_send_queue.front();
			game_data_send_queue.pop();
			LeaveCriticalSection(&CriticalSection_Send);
		}
		SecureZeroMemory(buffer, buffer_size);
		std::string hel = "Hello anyone there\n";
		strcpy_s(buffer, hel.c_str());
		
		int clientResult = sendto(*sock, buffer, buffer_size, 0 , (SOCKADDR*)&server_address, server_address_size);
		if (clientResult != 0)
		{
			//std::cout << "Data did not send, error: " << WSAGetLastError() << std::endl;
		}
		else if (clientResult == 0)
		{
			std::cout << "Send successful!" << std::endl;
		}
	}
}

int RunHostClient()
{
	int iResult;
	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	WSADATA WsaDat;

	// Initialise Windows Sockets
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
		return 0;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	iResult = getaddrinfo(NULL, "27015", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ListenSocket);

	//// Create a unbound socket.
	//SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//
	//if (Socket == INVALID_SOCKET)
	//{
	//	std::cout << "Socket creation failed!";
	//	WSACleanup();
	//	return 0;
	//}

	//// Now, try and bind the socket to any incoming IP address on Port 8888.
	//SOCKADDR_IN serverInf;

	//serverInf.sin_family = AF_INET;				// Address protocol family - internet protocol (IP addressing).
	//serverInf.sin_addr.s_addr = htonl(INADDR_ANY);	// Will accept any IP address from anywhere.
	//serverInf.sin_port = htons(27015);			// Port number - can change this, but needs to be the same on both client and server.
	//int bindResult = ::bind(Socket, (SOCKADDR*)(&serverInf), sizeof(serverInf));
	//if (bindResult == SOCKET_ERROR)
	//{
	//	std::cout << "Unable to bind socket!\r\n";
	//	WSACleanup();
	//	return 0;
	//}
	//else
	//{
	//	std::cout << "Bind Result: " << bindResult << std::endl;
	//}


	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids host");

	Game mGame;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;

	//InitializeCriticalSectionAndSpinCount(&CriticalSection_Recieve, 1000);
	//InitializeCriticalSectionAndSpinCount(&CriticalSection_Send, 1000);
	
	std::thread recieveThread(ReceiveFunction, &ClientSocket, &bRunning);
	//std::thread sendThread(SendFunction, &Socket, &bRunning);

	while (bRunning)
	{
		bRunning = window.isOpen();

		std::string data;

		sf::Event event;

		deltatime = clock.restart().asSeconds();

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

		}
		// Recieve data from client
		//EnterCriticalSection(&CriticalSection_Recieve);
		//if (game_data_recieve_queue.size() > 0)
		//{
		//	//mGame.UpdateGameData(game_data_recieve_queue.front());
		//	std::cout << game_data_recieve_queue.front() << std::endl;
		//	game_data_recieve_queue.pop();
		//}
		//LeaveCriticalSection(&CriticalSection_Recieve);	// Leave the critical section.

		
		mGame.Update(deltatime);

		window.clear(sf::Color::Black);
		mGame.Draw(window);

		
		window.display();

		// enter crit section
		/*EnterCriticalSection(&CriticalSection_Send);
		game_data_send_queue.push(mGame.SendGameData());
		LeaveCriticalSection(&CriticalSection_Send);*/
		// leave crit section

	}

	// Wait for the receiving and sending thread to complete.
	//sendThread.join();
	recieveThread.join();


	return 0;
}

int RunNormalClient()
{
	int iResult;
	// Initialise Winsock
	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "Winsock error - Winsock initialization failed\r\n";
		WSACleanup();
		return 0;
	}

	struct addrinfo *result = NULL,
					*ptr = NULL,
					hints;

	ZeroMemory(&hints, sizeof(hints));


	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo("localhost", "27015", &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	SOCKET connectSocket = INVALID_SOCKET;

	ptr = result;
	connectSocket = socket(ptr->ai_family, ptr->ai_socktype,
		ptr->ai_protocol);

	if (connectSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}


	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return 1;
	}
	//// Create our socket
	//SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//if (Socket == INVALID_SOCKET)
	//{
	//	std::cout << "Winsock error - Socket creation Failed!\r\n";
	//	WSACleanup();
	//	return 0;
	//}


	// Resolve IP address for hostname.
	//struct hostent* host;

	//// Change this to point to server, or ip address...
	//
	//if ((host = gethostbyname("localhost")) == NULL)   // In this case 'localhost' is the local machine. Change this to a proper IP address if connecting to another machine on the network.
	//{
	//	std::cout << "Failed to resolve hostname.\r\n";
	//	WSACleanup();
	//	return 0;
	//}

	

	
	// Sockets has now been initialised, so now can send some data to the server....


	const int buffer_size = 1024;
	char buffer[buffer_size];
	std::string connectedString = "Client connected!";
	strcpy_s(buffer, connectedString.c_str());

	// Send an initial buffer
	iResult = send(connectSocket, buffer, buffer_size, 0);
	if (iResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	// shutdown the connection since no more data will be sent
	iResult = shutdown(connectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(connectSocket);
		WSACleanup();
		return 1;
	}

	//struct sockaddr_in server_address;
	//int server_address_size = (int)sizeof(server_address);
	//short port = PORT;	// Port number - can change this, but needs to be the same on both client and server.
	//const char* server_ip_address = "127.0.0.1";	// The local host - change this for proper IP address of server if not on the local machine.
	//server_address.sin_family = AF_INET;
	//server_address.sin_port = htons(port);
	//server_address.sin_addr.s_addr = inet_addr(server_ip_address);

	//sockaddr addr;
	//memcpy(&addr, &server_address, server_address_size);

	//int result  = connect(Socket, &addr, sizeof(addr));
	//if (result != 0)
	//{
	//	std::cout << "Server unavaliable" << std::endl;
	//}
	//else
	//{
	//	std::cout << "Connected to server" << std::endl;
	//	memcpy(&server_address, &addr, sizeof(addr));

	//	strcpy_s(buffer, "Client connected!");
	//	sendto(Socket, buffer, buffer_size, 0, (SOCKADDR*)&server_address, server_address_size);
	//}

	Game mGame;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;

	
	InitializeCriticalSectionAndSpinCount(&CS_Client_Rcv, 1000);

	std::thread rcvThread(ClientRcv, &connectSocket, &bRunning);
	
	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids client");


	while (bRunning)
	{
		bRunning = window.isOpen();
		sf::Event event;

		deltatime = clock.restart().asSeconds();

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

		}
		//SecureZeroMemory(buffer, buffer_size);

		//// Get up to date data before game update loop
		//int bytes_received = recvfrom(Socket, buffer, buffer_size, 0, (SOCKADDR*)&server_address, &server_address_size);

		//if (bytes_received == SOCKET_ERROR)
		//{	// If there is an error, deal with it here...
		//	std::cout << "recvfrom failed with error " << WSAGetLastError();
		//}
		//else
		//{
		//	std::string vel = buffer;
		//	mGame.UpdateGameData(vel);
		//	//std::string acknowledge = buffer;
		//	std::cout << vel.c_str() << std::endl;
		//
		//}

		EnterCriticalSection(&CS_Client_Rcv);
		if (game_data_client_rcv_queue.size() > 0)
		{
			std::string vel = game_data_client_rcv_queue.front();

			//mGame.UpdateGameData(vel);

			std::cout << vel << std::endl;
			game_data_client_rcv_queue.pop();
		}
		LeaveCriticalSection(&CS_Client_Rcv);
		//-----------------------------------------------
		
		if (window.hasFocus())
		{
			mGame.Update(deltatime);

			window.clear(sf::Color::Black);
			mGame.Draw(window);
		}

		window.display();

		//SecureZeroMemory(buffer, buffer_size * sizeof(char));
		//// Send current game/frame data
		std::string posData = mGame.SendGameData();;
		strcpy_s(buffer, posData.c_str());
	//	int clientResult = sendto(Socket, buffer, buffer_size, 0, (SOCKADDR*)&server_address, server_address_size);
	}
	

	rcvThread.join();
	return 0;
}

int main()
{
	//sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids");

	/*Game mGame;

	sf::Clock clock;
	float deltatime = 0;*/

	char host = ' ';
	std::cout << "Is this the host client (y/n)" << std::endl;
	std::cin >> host;

	if (host == 'y')
	{
		RunHostClient();
	}
	else if (host == 'n')
		RunNormalClient();// Use normal client connection logic
	else
		std::cout << "Invalid command!" << std::endl;


	return 0;
}