#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <iostream>
#include "Game.h"

int main()
{
	sf::RenderWindow window(sf::VideoMode(1280, 720), "Asteroids");

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

		mGame.Update(deltatime);

		window.clear(sf::Color::Black);
		mGame.Draw(window);


		window.display();

	}

	return 0;
}