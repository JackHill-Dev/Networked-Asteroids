#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <synchapi.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <process.h>
#include <string>
#include <thread>
#include <mutex>
#include "Game.h"
#include "Network.h"

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
	std::string gData = "";
	const int buffer_size = 1024;
	char buffer[buffer_size];


	do 
	{
		//std::cout << "Host recieve thread running..." << std::endl;
		SecureZeroMemory(buffer, buffer_size);
		iResult = recv(*sock, buffer, buffer_size, 0);
		gData = buffer;
		if (iResult > 0) {

			EnterCriticalSection(&CriticalSection_Recieve);
			if (gData.find("\0") != std::string::npos)
			{
				std::cout << gData << std::endl;
				game_data_recieve_queue.push(gData);
			}

			//std::cout << gData << std::endl;
			LeaveCriticalSection(&CriticalSection_Recieve);
		}
		/*else if (iResult == 0)
			printf("Connection closing...\n");*/
		else
		{
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(*sock);
			WSACleanup();

		}
	
	} while (iResult > 0);


}

void ClientRcv(SOCKET* sock, bool* appRunning)
{
	SOCKET rcvSocket = *sock;

	const int buffer_size = 1024;
	char buffer[buffer_size];


	int iResult;
	std::string data = "";


	do 
	{
		//std::cout << "Recieve thread running" << std::endl;
		SecureZeroMemory(buffer, buffer_size);
		iResult = recv(*sock, buffer, buffer_size, 0);
		if (iResult > 0)
		{
			data = buffer;
			
			EnterCriticalSection(&CS_Client_Rcv);
			
			if (data.find("\0") != std::string::npos)
			{
				std::cout << data << std::endl;
				game_data_client_rcv_queue.push(data);
			}
			LeaveCriticalSection(&CS_Client_Rcv);
		}
		else if (iResult == 0)
		{
			std::cout << "Connection closed!" << std::endl;
		}
		else
			std::cout << "Rcv failed with error: " << WSAGetLastError() << std::endl;
	} while (iResult > 0);

	

	std::cout << "Out of loop" << std::endl;
}

void ClientSnd(SOCKET* sock, bool* appRunning)
{
	SOCKET sendSock = *sock;
	std::string pData = "";
	const int buffer_size = 1024;
	char buffer[buffer_size];
	SecureZeroMemory(&buffer, buffer_size);

	while (appRunning)
	{

		if (game_data_client_snd_queue.size() > 0)
		{
			EnterCriticalSection(&CS_Client_Snd);
			pData = game_data_client_snd_queue.front();
			game_data_client_snd_queue.pop();
			LeaveCriticalSection(&CS_Client_Snd);

		}

		if (pData != "")
		{
			strcpy_s(buffer, pData.c_str());
			int clientResult = send(sendSock, buffer, buffer_size, 0);//, 0, (SOCKADDR*)&server_address, server_address_size);
			if (clientResult == SOCKET_ERROR)
			{
				std::cout << "send() failed with error: " << WSAGetLastError() << std::endl;
			}
		}
	}

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

	u_long iMode = 0;
	int iResult;

	/*iResult = ioctlsocket(*sock, FIONBIO, &iMode);

	if (iResult != NOERROR)
		std::cout << "ioctlsocket failed with error: " << iResult;*/
	std::string gData = "";
	while (running)
	{
		EnterCriticalSection(&CriticalSection_Send);
		if (game_data_send_queue.size() > 0)
		{
			
			gData = game_data_send_queue.front();
			game_data_send_queue.pop();
			
		}
		LeaveCriticalSection(&CriticalSection_Send);

		SecureZeroMemory(buffer, buffer_size);
		strcpy_s(buffer, gData.c_str());
		
		int clientResult = send(*sock, buffer, buffer_size, 0);//, (SOCKADDR*)&server_address, server_address_size);
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

void HostAccept(SOCKET* listenSock, SOCKET* clientSock, int* noClients)
{
	int iResult;
	SOCKET acpt = *clientSock;
	SOCKET lstn = *listenSock;
	//while (*noClients != 2)
	//{
		// Accept client connections
	/*iResult = listen(lstn, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(lstn);
		WSACleanup();
		}*/
	int value = 1;
	// Accept a client socket
	//setsockopt(*clientSock, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));


	*clientSock = accept(*listenSock, NULL, NULL);
	if (*clientSock == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(*listenSock);
		WSACleanup();
		
	}
	else
	{
		(*noClients) += 1;
		//clientSock = &acpt;
		std::cout << "All Players connected!" << std::endl;
	}


		
	//}
	
	// Close listen socket
	closesocket(*listenSock);
}

void SetupConnectingScreen(sf::Font& fnt, sf::Text& txt)
{
	fnt.loadFromFile("Assets/Fonts/HISCORE.ttf");

	txt.setFont(fnt);
	txt.setCharacterSize(32);
	txt.setFillColor(sf::Color::White);
	
	txt.setPosition( 250, 300 );
	
	txt.setString("Waiting for clients to connect...");

}

int RunHostClient()
{
	int iResult;
	int clients = 1;
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
	
	std::thread acceptThread(HostAccept, &ListenSocket, &ClientSocket, &clients);

	
	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids host");

	Game mGame;
	Network serverNetwork;
	

	sf::Font fnt;
	sf::Text conTxt;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;
	bool gameStart = false;

	InitializeCriticalSectionAndSpinCount(&CriticalSection_Recieve, 1000);
	InitializeCriticalSectionAndSpinCount(&CriticalSection_Send, 1000);
	
	std::thread recieveThread;
	std::thread sendThread;

	SetupConnectingScreen(fnt, conTxt);
	bool keyPressed = false;
	while (bRunning)
	{
		bRunning = window.isOpen();

		if (clients == 2)
		{
			if (!recieveThread.joinable())
				recieveThread = std::thread(ReceiveFunction, &ClientSocket, &bRunning);
			

			if (!sendThread.joinable())
				sendThread = std::thread(SendFunction, &ClientSocket, &bRunning);


			gameStart = true;
		}

		std::string data;

		sf::Event event;

		deltatime = clock.restart().asSeconds();
		std::string command = " ";
		// Recieve data from clients
		EnterCriticalSection(&CriticalSection_Recieve);
		if (game_data_recieve_queue.size() > 0)
		{
			if (game_data_recieve_queue.front() != "")
			{
				mGame.UpdateGameData(deltatime, game_data_recieve_queue.front());
				game_data_recieve_queue = std::queue<std::string>();
			}
			
			//game_data_recieve_queue.pop();
		}
		LeaveCriticalSection(&CriticalSection_Recieve);	// Leave the critical section.
	
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			switch (event.type)
			{
			case sf::Event::KeyPressed:
				keyPressed = true;
				break;
			case sf::Event::KeyReleased:
				keyPressed = false;
				break;
			}
		}
		
		if (gameStart)
		{

			mGame.Update(deltatime);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && keyPressed)
			{

				mGame.mPlayer.move = Forward;
				command = "Forward";

			}
			
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) && keyPressed)
			{
				mGame.mPlayer.rotate = Left;
				command = "Left";
			}
			else
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) && keyPressed)
			{
				mGame.mPlayer.rotate = Right;
				command = "Right";
			}
			/*else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::D) && !sf::Keyboard::isKeyPressed(sf::Keyboard::A) && !keyPressed);
			{
				mGame.mPlayer.rotate = Still;
				command = "Still";
			}*/
			
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				command = "Fire";
			}
		
		
			window.clear(sf::Color::Black);
			mGame.Draw(window);

			EnterCriticalSection(&CriticalSection_Send);
			command += "\0";
			game_data_send_queue.push(command);
			LeaveCriticalSection(&CriticalSection_Send);

		}
		else
		{
			window.clear(sf::Color::Black);
			window.draw(conTxt);
		}
		

		window.display();

		
	}

	// Wait for the receiving and sending thread to complete.
	sendThread.join();
	recieveThread.join();
	acceptThread.join();

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

	u_long iMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR)
	{
		std::cout << "ioctSocket failed with error: " << WSAGetLastError() << std::endl;
	}

	char value = 1;
	setsockopt(connectSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

	Game mGame;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;

	
	InitializeCriticalSectionAndSpinCount(&CS_Client_Rcv, 1000);
	InitializeCriticalSectionAndSpinCount(&CS_Client_Snd, 1000);

	std::thread rcvThread(ClientRcv, &connectSocket, &bRunning);

	//std::thread sndThread(ClientSnd, &connectSocket, &bRunning);
	const int bufferSize = 1024;
	char buffer[bufferSize];
	std::string test = "ButtonPress with non blocking socket";
	strcpy_s(buffer, test.c_str());

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids client");

	std::string command = "";
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
		
		EnterCriticalSection(&CS_Client_Rcv);
		if (game_data_client_rcv_queue.size() > 0)
		{
			std::string command = game_data_client_rcv_queue.front();

			mGame.UpdateGameData(deltatime, command);
			std::cout << command << std::endl;
			game_data_recieve_queue = std::queue<std::string>();
			//game_data_client_rcv_queue.pop();
		}
		LeaveCriticalSection(&CS_Client_Rcv);
		//-----------------------------------------------
		
		
		mGame.Update(deltatime);


		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{

			mGame.mPlayer.move = Forward;
			command = "Forward";

		}
		/*else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::W))
		{
			mGame.mPlayer.move = Hold;
			command = "Hold";
		}*/
		else
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			mGame.mPlayer.rotate = Left;
			command = "Left";
		}
		else
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
		{
			mGame.mPlayer.rotate = Right;
			command = "Right";
		}
		/*else if (!sf::Keyboard::isKeyPressed(sf::Keyboard::D) || !sf::Keyboard::isKeyPressed(sf::Keyboard::A))
		{
			mGame.mPlayer.rotate = Still;
			command = "Still";
		}*/

		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			send(connectSocket, buffer, bufferSize, 0);
			//command = "Fire";


		window.clear(sf::Color::Black);
		mGame.Draw(window);
		

		window.display();

		
		EnterCriticalSection(&CS_Client_Snd);
		command += "\0";
		game_data_client_snd_queue.push(command);
		LeaveCriticalSection(&CS_Client_Snd);
	}
	

	rcvThread.join();
	//sndThread.join();
	return 0;
}

int main()
{

	char host = ' ';
	std::cout << "Is this the host client (y/n)" << std::endl;
	std::cin >> host;

	if (host == 'y')
	{
		RunHostClient();
	}
	else if (host == 'n')
		RunNormalClient(); // Use normal client connection logic
	else
		std::cout << "Invalid command!" << std::endl;


	return 0;
}