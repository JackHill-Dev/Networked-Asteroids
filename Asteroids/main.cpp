#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <queue>
#include <stdio.h>
#include <process.h>
#include <string>

#include "Game.h"

#pragma comment(lib, "ws2_32.lib")	// Use this library whilst linking - contains the Winsock2 implementation.

CRITICAL_SECTION CriticalSection;

std::queue<std::string> data_queue;

unsigned __stdcall ReceiveFunction(void* pArguments)
{


	std::cout << "Starting Winsock on thread." << std::endl;

	WSADATA WsaDat;

	// Initialise Windows Sockets
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "WSA Initialization failed!\r\n";
		WSACleanup();
		return 0;
	}

	// Create a unbound socket.
	SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (Socket == INVALID_SOCKET)
	{
		std::cout << "Socket creation failed!";
		WSACleanup();
		return 0;
	}

	// Now, try and bind the socket to any incoming IP address on Port 8888.
	SOCKADDR_IN serverInf;

	serverInf.sin_family = AF_INET;				// Address protocol family - internet protocol (IP addressing).
	serverInf.sin_addr.s_addr = htonl(INADDR_ANY);	// Will accept any IP address from anywhere.
	serverInf.sin_port = htons(49152);			// Port number - can change this, but needs to be the same on both client and server.

	if (bind(Socket, (SOCKADDR*)(&serverInf), sizeof(serverInf)) == SOCKET_ERROR)
	{
		std::cout << "Unable to bind socket!\r\n";
		WSACleanup();
		return 0;
	}

	// Now Sockets have been initialised, so now wait for some data from a client...

	bool finished = false;

	// Repeat 
	while (!finished)
	{
		const int buffer_size = 1024;// sizeof(sf::Vector2f);
		char buffer[buffer_size];	// Space for the data.

		struct sockaddr_in client_address;	// Placeholder for client address information - 'recvfrom' will fill this in and return the size of this data in client_address_size.
		int client_address_size = sizeof(client_address);

		std::cout << "Waiting for data from client..." << std::endl;

		int bytes_received = recvfrom(Socket, buffer, buffer_size, 0, (SOCKADDR*)&client_address, &client_address_size);

		if (bytes_received == SOCKET_ERROR)
		{	// If there is an error, deal with it here...
			std::cout << "'recvfrom' failed with error " << WSAGetLastError();
			std::cout << std::endl;
		}
		else
		{
			// No error, so put the data on the queue for the main thread.
			std::string t = buffer;	// t contains the string sent here from the current client.

			if (t == "quit") finished = true;	// Message from the main thread to terminate this thread.

			EnterCriticalSection(&CriticalSection);
			data_queue.push(t);
			LeaveCriticalSection(&CriticalSection);

			// Send something back to the client as an acknowledgement...
			std::string data_to_send = "Message Received and Understood.";

			sf::Vector2f position(10, 20);
			std::string posToSend = "Pos" + std::to_string(position.x) + "/" + std::to_string(position.y);
		

			strcpy_s(buffer, posToSend.c_str());
			
			sendto(Socket, buffer, buffer_size, 0, (SOCKADDR*)&client_address, client_address_size);
		}
	}

	// Shutdown the socket.
	shutdown(Socket, SD_SEND);

	// Close our socket entirely.
	closesocket(Socket);

	// Cleanup Winsock - release any resources used by Winsock.
	WSACleanup();

	_endthreadex(0);
	return 0;
}

int RunHostClient()
{
	HANDLE hThread;
	unsigned threadID;

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids host");

	Game mGame;

	sf::Clock clock;
	float deltatime = 0;

	InitializeCriticalSectionAndSpinCount(&CriticalSection, 1000);
	// Create revieve function thread
	hThread = (HANDLE)_beginthreadex(NULL, 0, &ReceiveFunction, NULL, 0, &threadID);

	while (window.isOpen())
	{
		std::string data;

		sf::Event event;

		deltatime = clock.restart().asSeconds();

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

		}

		EnterCriticalSection(&CriticalSection);
		if (data_queue.size() > 0)
		{
			data = data_queue.front();	// Retrieve the next data item from the queue.
			data_queue.pop();			// Eliminate it from the queue.

			std::cout << "Data Received:" << data;
			std::cout << std::endl;
		}
		LeaveCriticalSection(&CriticalSection);	// Leave the critical section.

		if (window.hasFocus())
		{
			mGame.Update(deltatime);

			window.clear(sf::Color::Black);
			mGame.Draw(window);

		}
		window.display();

	}

	// Wait for the receiving thread to complete.
	WaitForSingleObject(hThread, INFINITE);

	// Clean up...
	CloseHandle(hThread);

	return 0;
}

int RunNormalClient()
{
	// Initialise Winsock
	WSADATA WsaDat;
	if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	{
		std::cout << "Winsock error - Winsock initialization failed\r\n";
		WSACleanup();
		return 0;
	}

	// Create our socket
	SOCKET Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (Socket == INVALID_SOCKET)
	{
		std::cout << "Winsock error - Socket creation Failed!\r\n";
		WSACleanup();
		return 0;
	}

	// Resolve IP address for hostname.
	struct hostent* host;

	// Change this to point to server, or ip address...

	if ((host = gethostbyname("localhost")) == NULL)   // In this case 'localhost' is the local machine. Change this to a proper IP address if connecting to another machine on the network.
	{
		std::cout << "Failed to resolve hostname.\r\n";
		WSACleanup();
		return 0;
	}


	// Sockets has now been initialised, so now can send some data to the server....


	const int buffer_size = 1024;
	char buffer[1024];
	SecureZeroMemory(&buffer, buffer_size);

	struct sockaddr_in server_address;
	int server_address_size = (int)sizeof(server_address);
	short port = 49152;	// Port number - can change this, but needs to be the same on both client and server.
	const char* server_ip_address = "127.0.0.1";	// The local host - change this for proper IP address of server if not on the local machine.
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = inet_addr(server_ip_address);

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids client");

	Game mGame;

	sf::Clock clock;
	float deltatime = 0;

	while (window.isOpen())
	{
		sf::Event event;

		deltatime = clock.restart().asSeconds();

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

		}

		// Get up to date date before game update loop
		int bytes_received = recvfrom(Socket, buffer, buffer_size, 0, (SOCKADDR*)&server_address, &server_address_size);

		if (bytes_received == SOCKET_ERROR)
		{	// If there is an error, deal with it here...
			std::cout << "recvfrom failed with error " << WSAGetLastError();
		}
		else
		{
			std::string acknowledge = buffer;
			std::cout << acknowledge.c_str() << std::endl;
			// At this point we have received an acknowledgement from the server, so we can carry on...
		}
		//-----------------------------------------------
		if (window.hasFocus())
		{
			mGame.Update(deltatime);

			window.clear(sf::Color::Black);
			mGame.Draw(window);
		}

		window.display();


		// Send current game/frame data
		std::string testData = "Does the queue work now!";
		strcpy_s(buffer, testData.c_str());
		int clientResult = sendto(Socket, buffer, buffer_size, 0, (SOCKADDR*)&server_address, server_address_size);
	}



	// Sent the data, now wait for acknowledgement coming back...

	

	// Shutdown our socket.
	shutdown(Socket, SD_SEND);

	// Close our socket entirely.
	closesocket(Socket);
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


	return 0;
}