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
		spr.setPosition(1380, 800);
		velocity = { 0,0 };
	}

	bool bInPool = true;
	sf::Vector2f velocity;
	sf::Sprite spr;
	sf::Texture texture;

};

struct Asteroid
{
public:

	Asteroid()
	{
		texture.loadFromFile("asteroid.png");
		spr = sf::Sprite(texture);
	}

	int health = 5;
	sf::Vector2f velocity = {0.1,0.1};
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
	void WrapObject(sf::Sprite& spr);
	void DisableBullet(Bullet& bullet);
	void FireBullet(Bullet& bullet, const float& deltatime);
	sf::Clock shootClock;
	sf::Time lastFired;
	std::vector<Bullet> bullets;

	std::vector<Asteroid> asteroids;
	Asteroid a1;
	Asteroid a2;

	sf::Texture asteroidTexture;
	Player mPlayer;

	int width,height;
};

