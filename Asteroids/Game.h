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
	bool isDestroyed = false;
	sf::Vector2f velocity = {0.1,0.1};
	sf::Sprite spr;	
	sf::Texture texture;
	
};

typedef int PlayerID;

class Game
{
public:
	Game(const bool& host);
	~Game();
	void Update(const float& deltaTime);
	void Draw(sf::RenderWindow& wnd);

	void UpdateGameData(float& dt, char* gData);


	std::string SendGameData();
	std::string SendAsteroidData();
	std::string SendPlayerInfoData();

	float ToRadians(float x)
	{
		return	x * (3.14 / 180);
	}
	Player mPlayer;
	bool GameOver();
	void CreatePlayerPosPacket();
	void CreateAsteroidPacket();
private:
	void UpdateCollisions(const float& deltaTime);
	void Shoot(const float& dt, Player& originPlayer, std::vector<Bullet*>& bulletPool);
	void WrapObject(sf::Sprite& spr);
	void DisableBullet(Bullet* bullet);
	void FireBullet(Player& originPlayer, Bullet* bullet, const float& deltatime);
	void SetTextProperties(sf::Text& txt, const sf::Vector2f& pos);


	int RandomNumberGenerator(int min, int max);

	void DeserialiseAsteroidData(std::string& astData);
	void DeserialisePlayerData(std::string& pData);
	std::string BoolToString(bool& b);
	float FindNextValue(std::string& str, int charAmount, char seperator);

	sf::Clock shootClock;
	sf::Time lastFired;

	std::vector<Bullet*> bullets;
	std::vector<Bullet*> player2Bullets;
	std::vector<Asteroid*> asteroids;

	std::map<PlayerID, Player> players;
	//ClientNetwork* server;


	Player mPlayer2;

	// Text objects
	sf::Text mPlayer1Txt;
	sf::Text mPlayer1LivesTxt;
	sf::Text mPlayer2Txt;
	sf::Text mPlayer2LivesTxt;

	sf::Text mPlayer1WinTxt;
	bool displayP1WinTxt = false;
	sf::Text mPlayer2WinTxt;
	bool displayP2WinTxt = false;


	sf::Font scoreFnt;

	int width,height;
	bool bGameover = false;
	bool isHost = false;
};

