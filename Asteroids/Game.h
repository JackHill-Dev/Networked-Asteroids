#pragma once
#include "Player.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <iostream>
#include <random>
#include <chrono>


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

	Asteroid(sf::Vector2f startPos, sf::Vector2f startVel)
	{
		texture.loadFromFile("asteroid.png");
		spr = sf::Sprite(texture);
		spr.setPosition(startPos);
		spr.setOrigin(32, 32);
		velocity = startVel;
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
	~Game();
	void Init();
	void Update(const float& deltaTime);
	void Draw(sf::RenderWindow& wnd);
	void UpdateGameData(std::string& gData);
	std::string SendGameData();
	float ToRadians(float x)
	{
		return	x * (3.14 / 180);
	}
	Player mPlayer;

private:
	void UpdateCollisions(const float& deltaTime);
	void Shoot(const float& dt);
	void WrapObject(sf::Sprite& spr);
	void DisableBullet(Bullet* bullet);
	void FireBullet(Bullet* bullet, const float& deltatime);
	void SetTextProperties(sf::Text& txt, const sf::Vector2f& pos);
	void GameOver();
	int RandomNumberGenerator(int min, int max);

	sf::Clock shootClock;
	sf::Time lastFired;

	std::vector<Bullet*> bullets;
	std::vector<Asteroid*> asteroids;


	Player mPlayer2;

	// Text objects
	sf::Text mPlayer1Txt;
	sf::Text mPlayer1LivesTxt;
	sf::Text mPlayer2Txt;
	sf::Text mPlayer2LivesTxt;

	sf::Font scoreFnt;

	int width,height;
	bool bGameover = false;
};

