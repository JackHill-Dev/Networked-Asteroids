#pragma once
#include "Player.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>

struct Bullet
{
public:

	Bullet()
	{
		texture.loadFromFile("bullet.png");
		spr = sf::Sprite(texture);
	}

	sf::Vector2f velocity;
	sf::Sprite spr;
	sf::Texture texture;

};

class Game
{
public:
	Game();
	void Init();
	void Update(const float& deltaTime);
	void Draw(sf::RenderWindow& wnd);

	float ToRadians(float x)
	{
		return	x * (3.14 / 180);
	}


private:
	void UpdateCollisions(const float& deltaTime);
	void Shoot(const float& dt);
	void WrapPlayer();
	Bullet CreateBullet(const sf::Sprite& player, const float& deltatime);
	sf::Clock shootClock;
	sf::Time lastFired;
	std::vector<Bullet> bullets;
	std::vector<sf::Sprite> asteroids;
	sf::Texture asteroidTexture;
	Player mPlayer;

	int width,height;
};

