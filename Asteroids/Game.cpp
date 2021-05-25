#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
std::mt19937 random_number_engine(time(0));
Game::Game(const bool& host)
{
	isHost = host;

	scoreFnt.loadFromFile("Assets/Fonts/HISCORE.ttf");

	mPlayer2.SetSpriteColor(sf::Color::Red);

	SetTextProperties(mPlayer1Txt, { 10,0 });
	SetTextProperties(mPlayer1LivesTxt, { 10,20 });

	SetTextProperties(mPlayer2Txt, { 1120,0 });
	SetTextProperties(mPlayer2LivesTxt, { 1120,20 });

	bullets.reserve(100);
	player2Bullets.reserve(100);
	asteroids.reserve(50);

	// Fill bullets pool
	for (int i = 0; i < 100; ++i)
	{
		bullets.push_back(new Bullet());
	}

	
	for (int i = 0; i < 100; ++i)
	{
		player2Bullets.push_back(new Bullet());
	}

	for (auto& b : player2Bullets)
	{
		b->spr.setColor(sf::Color::Yellow);
	}

	// TODO: possibly remove
	if (isHost)
	{
		for (int i = 0; i < 10; ++i)
		{
			sf::Vector2f velocity = sf::Vector2f(RandomNumberGenerator(-50, 100), RandomNumberGenerator(-50, 100));
			sf::Vector2f startPos = sf::Vector2f(RandomNumberGenerator(0, 1280), RandomNumberGenerator(0, 720));
			asteroids.push_back(new Asteroid(startPos, velocity));
		}
	}
	else
	{
		for (int i = 0; i < 10; ++i)
		{
			sf::Vector2f velocity = sf::Vector2f(0,0);
			sf::Vector2f startPos = sf::Vector2f(-1380, 800);
			asteroids.push_back(new Asteroid(startPos, velocity));
		}
	}

}

Game::~Game()
{
	for (auto b : bullets)
	{
		delete b;
		b = nullptr;
	
	}

	for (auto b : player2Bullets)
	{
		delete b;
		b = nullptr;

	}

	for (auto a : asteroids)
	{
		delete a;
		a = nullptr;

	}
}

void Game::Update(const float& deltaTime)
{
	mPlayer.Update(deltaTime);
	//mPlayer2.Update(deltaTime);
	

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		Shoot(deltaTime, mPlayer, bullets);
	}

	for (auto& b : bullets)
	{
		b->spr.move(b->velocity);
		
		DisableBullet(b); // Checks if bullets are out of range and put
	}

	for (auto& b : player2Bullets)
	{
		b->spr.move(b->velocity);

		DisableBullet(b); // Checks if bullets are out of range and put it back into pool in a static state
	}

	if (isHost)
	{
		for (auto& a : asteroids)
		{
			a->spr.rotate(50 * deltaTime);
			a->spr.move(a->velocity * deltaTime);
			WrapObject(a->spr);
		}
	}
	else
	{
		for (auto& a : asteroids)
		{
			WrapObject(a->spr);
		}
	}

	UpdateCollisions(deltaTime);
	WrapObject(mPlayer.GetSprite());
	WrapObject(mPlayer2.GetSprite());
	

	if (asteroids.empty())
	{
		GameOver();
	}

}

void Game::Draw(sf::RenderWindow& wnd)
{
	wnd.draw(mPlayer.GetSprite());

	wnd.draw(mPlayer2.GetSprite());

	for (auto& b : bullets)
	{
		wnd.draw(b->spr);
	}

	for (auto& b : player2Bullets)
	{
		wnd.draw(b->spr);
	}

	for (auto& a : asteroids)
	{
		wnd.draw(a->spr);
	}

	mPlayer1Txt.setString("P1: " + std::to_string( mPlayer.score));
	mPlayer1LivesTxt.setString("Lives: " + std::to_string( mPlayer.lives));

	mPlayer2Txt.setString("P2: " + std::to_string( mPlayer2.score));
	mPlayer2LivesTxt.setString("Lives: " + std::to_string( mPlayer2.lives));

	wnd.draw(mPlayer1Txt);
	wnd.draw(mPlayer1LivesTxt);
	wnd.draw(mPlayer2Txt);
	wnd.draw(mPlayer2LivesTxt);
}

void Game::UpdateGameData(float& dt, std::string& gData)
{
	// int id = 1;
	// players[id].DesrializeData(gData);

	mPlayer2.DesrializeData(gData);

	if(!isHost)
		DeserialiseAsteroidData(gData);

	if (gData == "Fire")
		Shoot(dt, mPlayer2, player2Bullets);

}

void Game::DeserialiseAsteroidData(std::string& astData)
{
	float x, y, r;
	int elementPos;
	bool destroyed = false;
	int pos = 0;
	std::string result;
	bool finished = false;
	if (strstr(astData.c_str(), "Asteroids"))
	{
		pos = astData.find_first_of('[');
		astData.erase(astData.begin(), astData.begin() + pos + 2);
		//pos++;

		
		while ( !astData.empty() && astData.front() != ']')
		{
			if (astData[pos] != ',')
			{
				// Find element position
				elementPos = FindNextValue(astData, 1, '/');
				// convert element position from string to int
				
				// Find destroyed state
				destroyed = FindNextValue(astData, 1, '/');
				// Find all positional information
				x = FindNextValue(astData, 7, '.');
				y = FindNextValue(astData, 7, '.');
				r = FindNextValue(astData, 7, '.');

				// Apply updated information to specific asteroid
				asteroids[elementPos]->spr.setPosition(x, y);
				asteroids[elementPos]->spr.setRotation(r);
				asteroids[elementPos]->isDestroyed = destroyed;

				
				astData.erase(astData.begin());
			
			}
	
			
		}

	}
}

float Game::FindNextValue(std::string& str, int charAmount, char seperator)
{
	std::string temp;
	float val = 0;
	int pos = 0;

	if (charAmount == 7)
	{
		if (str.front() == '/')
			str.erase(str.begin());

		while (str[pos] != seperator)
		{
			temp += str[pos++];
		}

		temp += str.substr(pos, pos + charAmount);

		val = std::stof(temp);

		str.erase(str.begin(), str.begin() + charAmount + pos);
	}
	else
	{
		if (str.front() == ',')
			str.erase(str.begin());

		if (str.front() == '/')
			str.erase(str.begin());

		while (str[pos] != seperator)
		{
			temp += str[pos++];
		}

		val = std::stof(temp);

		str.erase(str.begin(), str.begin() + charAmount + 1);
	}


	return val;
}


std::string Game::SendGameData()
{

	std::string posPacket = isHost ? mPlayer.SerializeData() + SendAsteroidData() : mPlayer.SerializeData() ;

	return posPacket;
}



std::string Game::SendAsteroidData()
{
	char seperator = '/';
	float x, y, r;
	int elementPos = 0;

	std::string asteroidsStr;


	asteroidsStr += "Asteroids[";


	for (auto& a : asteroids)
	{
		x = a->spr.getPosition().x;
		y = a->spr.getPosition().y;
		r = a->spr.getRotation();

		// Contruct asteroid string
		// [x/y/r/elementPos/bDestroyed],
		asteroidsStr += std::to_string(elementPos) + seperator +								 // Element position in vector
			BoolToString(a->isDestroyed) + seperator  +									 // destroyed state
			std::to_string(x) + seperator + std::to_string(y) + seperator + std::to_string(r) + ',';	 // Positional data



		++elementPos;
	}

	asteroidsStr += ']';

	return asteroidsStr;
}

std::string Game::SendPlayerInfoData()
{
	std::string pData = "Info/" + std::to_string(mPlayer.score) + "/" + std::to_string(mPlayer.lives) + "/\0";

	return pData;
}

void Game::Shoot(const float& dt, Player& originPlayer, std::vector<Bullet*>& asteroidPool)
{
	sf::Time now = shootClock.getElapsedTime();
	if ((now - lastFired) >= originPlayer.GetShootDelay())
	{
		lastFired = now;
		auto bullet = std::find_if(asteroidPool.begin(), asteroidPool.end(), [](Bullet* b) {return b->bInPool; });
		FireBullet(originPlayer, *bullet, dt);

	}

}

void Game::WrapObject(sf::Sprite& sprite)
{
	// Wrap player to opposite side of screen
	// TODO: Remove magic numbers for the screen width and height
	if (sprite.getPosition().x > 1280) sprite.setPosition(0, sprite.getPosition().y);
	if (sprite.getPosition().x < 0) sprite.setPosition(1280, sprite.getPosition().y);
	if (sprite.getPosition().y > 720) sprite.setPosition(sprite.getPosition().x, 0);
	if (sprite.getPosition().y < 0) sprite.setPosition(sprite.getPosition().x, 720);

}

void Game::DisableBullet(Bullet* bullet)
{
	if ((bullet->spr.getPosition().x > 1280) ||
		(bullet->spr.getPosition().x < 0) ||
		(bullet->spr.getPosition().y > 720) ||
		(bullet->spr.getPosition().y < 0))
	{
		bullet->bInPool = true;
		bullet->velocity = { 0, 0 };
		bullet->spr.setPosition(1380, 800); // Object pool position
	}

}

void Game::FireBullet(Player& originPlayer, Bullet* bullet, const float& deltatime)
{

	bullet->spr.setOrigin(4, 4);
	bullet->spr.setPosition(originPlayer.GetSprite().getPosition().x, originPlayer.GetSprite().getPosition().y);
	bullet->spr.setRotation(originPlayer.GetSprite().getRotation());

	bullet->velocity.x = sin(ToRadians(bullet->spr.getRotation())) * 450.f * deltatime;
	bullet->velocity.y = -cos(ToRadians(bullet->spr.getRotation())) * 450.f * deltatime;

	bullet->bInPool = false;
	

}

void Game::SetTextProperties(sf::Text& txt, const sf::Vector2f& pos)
{
	txt.setFont(scoreFnt);
	txt.setCharacterSize(24);
	txt.setFillColor(sf::Color::White);
	txt.setStyle(sf::Text::Bold);
	txt.setPosition(pos);
}

void Game::GameOver()
{
	// Handle game over logic
}

int Game::RandomNumberGenerator(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(random_number_engine);
}

std::string Game::BoolToString(bool& b)
{
	if (b)
		return "1";
	else
		return "0";
}


void Game::UpdateCollisions(const float& deltaTime)
{
	//// Check bullet collison with asteroids
	for (auto& a : asteroids)
	{
		
		for (auto& b : bullets)
		{
			if (a->spr.getGlobalBounds().intersects(b->spr.getGlobalBounds()))
			{
				// Destroy asteroid
				asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(), [&a](Asteroid* ast) { return a->spr.getPosition() == ast->spr.getPosition(); }));
				mPlayer.AddScore(10);
				b->bInPool = true;
				b->velocity = { 0, 0};
				b->spr.setPosition(1380, 800); // Object pool position

			}
		}

		// Check player collision with asteroid
		if (a->spr.getGlobalBounds().intersects(mPlayer.GetSprite().getGlobalBounds()))
		{
			//if (mPlayer.lives > 0)
			//{
			//	// Lose life
			//	mPlayer.RemoveLife();
			//	// Respawn at center of screen in an invulnerable state for a second
			//	mPlayer.GetSprite().setPosition({ 640, 360 });
			//}
			
				
		}
	}
}

