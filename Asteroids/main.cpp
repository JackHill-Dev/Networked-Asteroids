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
#include <thread>
#include "Game.h"

#pragma comment(lib, "ws2_32.lib")	// Use this library whilst linking - contains the Winsock2 implementation.

CRITICAL_SECTION CriticalSection_Recieve;
CRITICAL_SECTION CriticalSection_Send;

std::queue<std::string> game_data_recieve_queue;
std::queue<std::string> game_data_send_queue;

void ReceiveFunction(SOCKET* sock, bool* running)
{
	std::cout << "Starting Winsock on thread." << std::endl;
	//bool finished = false;

	// Repeat 
	while (running)
	{
		const int buffer_size = 1024;// sizeof(sf::Vector2f);
		char buffer[buffer_size];	// Space for the data.

		struct sockaddr_in client_address;	// Placeholder for client address information - 'recvfrom' will fill this in and return the size of this data in client_address_size.
		int client_address_size = sizeof(client_address);

		std::cout << "Waiting for data from client..." << std::endl;

		int bytes_received = recvfrom(*sock, buffer, buffer_size, 0, (SOCKADDR*)&client_address, &client_address_size);

		if (bytes_received == SOCKET_ERROR)
		{	// If there is an error, deal with it here...
			std::cout << "'recvfrom' failed with error " << WSAGetLastError();
			std::cout << std::endl;
		}
		else
		{
			// No error, so put the data on the queue for the main thread.
			std::string t = buffer;	// t contains the string sent here from the current client.

			if (t == "quit") *running = true;	// Message from the main thread to terminate this thread.

			EnterCriticalSection(&CriticalSection_Recieve);
			game_data_recieve_queue.push(t);
			LeaveCriticalSection(&CriticalSection_Recieve);

		}
	}
	
}

void SendFunction(SOCKET* sock, bool* running)
{
	const int buffer_size = 1024;
	char buffer[1024];
	SecureZeroMemory(&buffer, buffer_size);

	struct sockaddr_in server_address;
	int server_address_size = (int)sizeof(server_address);
	short port = 49152;	// Port number - can change this, but needs to be the same on both client and server.
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

		strcpy_s(buffer, gData.c_str());
		int clientResult = sendto(*sock, buffer, buffer_size, 0, (SOCKADDR*)&server_address, server_address_size);
	}
}

int RunHostClient()
{

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
	serverInf.sin_port = htons(49153);			// Port number - can change this, but needs to be the same on both client and server.

	if (::bind(Socket, (SOCKADDR*)(&serverInf), sizeof(serverInf)) == SOCKET_ERROR)
	{
		std::cout << "Unable to bind socket!\r\n";
		WSACleanup();
		return 0;
	}


	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids host");

	Game mGame;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;

	InitializeCriticalSectionAndSpinCount(&CriticalSection_Recieve, 1000);
	InitializeCriticalSectionAndSpinCount(&CriticalSection_Send, 1000);
	
	std::thread recieveThread(ReceiveFunction, &Socket, &bRunning);
	std::thread sendThread(SendFunction, &Socket, &bRunning);

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
		EnterCriticalSection(&CriticalSection_Recieve);
		if (game_data_recieve_queue.size() > 0)
		{
			mGame.UpdateGameData(game_data_recieve_queue.front());
			game_data_recieve_queue.pop();
		}
		LeaveCriticalSection(&CriticalSection_Recieve);	// Leave the critical section.

		
		mGame.Update(deltatime);

		window.clear(sf::Color::Black);
		mGame.Draw(window);

		
		window.display();

		// enter crit section
		EnterCriticalSection(&CriticalSection_Send);
		game_data_send_queue.push(mGame.SendGameData());
		LeaveCriticalSection(&CriticalSection_Send);
		// leave crit section

	}

	// Wait for the receiving and sending thread to complete.
	sendThread.join();
	recieveThread.join();


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
	short port = 49153;	// Port number - can change this, but needs to be the same on both client and server.
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
			std::string vel = buffer;
			mGame.UpdateGameData(vel);
			//std::string acknowledge = buffer;
			std::cout << vel.c_str() << std::endl;
		
		}
		//-----------------------------------------------
		
		mGame.Update(deltatime);

		window.clear(sf::Color::Black);
		mGame.Draw(window);
		

		window.display();

		
		// Send current game/frame data
		std::string posData = mGame.SendGameData();;
		strcpy_s(buffer, posData.c_str());
		int clientResult = sendto(Socket, buffer, buffer_size, 0, (SOCKADDR*)&server_address, server_address_size);
	}

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