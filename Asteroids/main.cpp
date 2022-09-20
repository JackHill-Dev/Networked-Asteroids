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
#include <string>
#include <thread>
#include <mutex>
#include "Game.h"
#include "Network.h"

//#pragma comment(lib, "ws2_32.lib")	// Use this library whilst linking - contains the Winsock2 implementation.

void SetupConnectingScreen(sf::Font& fnt, sf::Text& txt)
{
	fnt.loadFromFile("Assets/Fonts/HISCORE.ttf");

	txt.setFont(fnt);
	txt.setCharacterSize(32);
	txt.setFillColor(sf::Color::White);
	
	txt.setPosition( 250, 300 );
	
	txt.setString("Waiting for clients to connect...");

}

void RunHostClient()
{

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids host");

	Game mGame(true);
	Network serverNetwork;
	
	sf::Font fnt;
	sf::Text conTxt;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;
	bool gameStart = false;

	mGame.CreateAsteroidPacket();
	SetupConnectingScreen(fnt, conTxt);
	bool keyPressed = false;
	while (bRunning)
	{
		bRunning = window.isOpen();

		if (serverNetwork.AllClientsConnected())
		{
			gameStart = true;
		}

		std::string data;

		sf::Event event;

		deltatime = clock.restart().asSeconds();
		std::string command = " ";

		// Recieve data from clients
		if (serverNetwork.rcvQueue.size() > 0)
		{
			serverNetwork.rcvMutex.lock();
		
			mGame.UpdateGameData(deltatime, serverNetwork.rcvQueue.front());
			serverNetwork.rcvQueue = std::queue<std::string>(); //.pop();

			serverNetwork.rcvMutex.unlock();
		}
	

	
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			
		}
		
		if (gameStart)
		{
			mGame.Update(deltatime);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				serverNetwork.Send("Fire");
			}


			window.clear(sf::Color::Black);
			mGame.Draw(window);

			serverNetwork.Send(mGame.SendGameData().c_str());
		
		}
		else
		{
			window.clear(sf::Color::Black);
			window.draw(conTxt);
		}
		
		window.display();


		
	}

	serverNetwork.rcv.join();

}


void RunNormalClient(std::string& ip)
{

	Game mGame(false);
	ClientNetwork clientNetwork(ip);

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;

	std::string test = "Connection request";
	clientNetwork.Send(test.c_str());

	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids client");

	std::string command = "";
	bool gameStart = false;



	while (bRunning)
	{
		bRunning = window.isOpen();

		sf::Event event;

		deltatime = clock.restart().asSeconds();


		if (clientNetwork.rcvQueue_Client.size() > 0)
		{
			clientNetwork.rcvMutex_Client.lock();
			
			mGame.UpdateGameData(deltatime, clientNetwork.rcvQueue_Client.front());
			clientNetwork.rcvQueue_Client = std::queue<std::string>();//.pop();

			clientNetwork.rcvMutex_Client.unlock();
		}

		if (clientNetwork.GetID() == 2 && !gameStart)
			gameStart = true;

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				clientNetwork.Send("Disconnect");
				window.close();

			
			}

		}

		if (gameStart)
		{
			mGame.Update(deltatime);

			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			{
				clientNetwork.Send("Fire");
			}

			window.clear(sf::Color::Black);
			mGame.Draw(window);

			clientNetwork.Send(mGame.SendGameData().c_str());
	

		}
		else
			window.clear(sf::Color::Red);
		
		window.display();


		


	}

	clientNetwork.rcv_Client.join();
	
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
	{
		std::string ip;
		std::cout << "Enter host ip (ipv4): " << std::endl;
		std::cin >> ip;
		RunNormalClient(ip); // Use normal client connection logic
	}
	else
		std::cout << "Invalid command!" << std::endl;


	return 0;
}