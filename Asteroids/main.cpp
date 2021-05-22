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

	Game mGame;
	Network serverNetwork;
	
	sf::Font fnt;
	sf::Text conTxt;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;
	bool gameStart = false;

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
		serverNetwork.rcvMutex.lock();
		if (serverNetwork.rcvQueue.size() > 0)
		{
			mGame.UpdateGameData(deltatime, serverNetwork.rcvQueue.front());
			serverNetwork.rcvQueue.pop();
		}
		serverNetwork.rcvMutex.unlock();

	
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

		}
		else
		{
			window.clear(sf::Color::Black);
			window.draw(conTxt);
		}
		
		window.display();

	}

}


void RunNormalClient()
{

	Game mGame;
	ClientNetwork clientNetwork;

	sf::Clock clock;
	float deltatime = 0;

	bool bRunning = true;

	std::string test = "Connection request";
	clientNetwork.Send(test.c_str());


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
			{
				clientNetwork.Send("Disconnect");
				window.close();
			}

		}

		clientNetwork.rcvMutex_Client.lock();
		if (clientNetwork.rcvQueue_Client.size() > 0)
		{
			mGame.UpdateGameData(deltatime, clientNetwork.rcvQueue_Client.front());
		}
		clientNetwork.rcvMutex_Client.unlock();



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

		//if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		//	send(connectSocket, buffer, bufferSize, 0);
			//command = "Fire";


		window.clear(sf::Color::Black);
		mGame.Draw(window);


		window.display();


		
	}
	
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