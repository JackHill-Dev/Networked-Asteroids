#include "Game.h"
#include "Messages.h"
#include <SFML/Window/Keyboard.hpp>
std::mt19937 random_number_engine(time(0));
Game::Game(const bool& host)
{
	isHost = host;

	scoreFnt.loadFromFile("Assets/Fonts/HISCORE.ttf");

	mPlayer2.SetSpriteColor(sf::Color::Red);

	SetTextProperties(mPlayer1Txt, { 10,0 });

	SetTextProperties(mPlayer2Txt, { 1120,0 });

	SetTextProperties(mPlayer1WinTxt, { 500,300 });
	SetTextProperties(mPlayer2WinTxt, { 500,300 });

	mPlayer1WinTxt.setString("Player 1 Wins a sock!");

	mPlayer2WinTxt.setString("Player 2 Wins a sock!");

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
	mPlayer2.Update(deltaTime);
	

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
	

	UpdateCollisions(deltaTime);
	WrapObject(mPlayer.GetSprite());
	WrapObject(mPlayer2.GetSprite());
	

	
	GameOver();
	

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

	wnd.draw(mPlayer2Txt);

	if (displayP1WinTxt)
		wnd.draw(mPlayer1WinTxt);

	if (displayP2WinTxt)
		wnd.draw(mPlayer2WinTxt);
}

void Game::UpdateGameData(float& dt,  char* buffer)
{

	/*

	if(!isHost)
		DeserialiseAsteroidData(gData);

	DeserialisePlayerData(gData);

	if (gData == "Fire")
	{
		Shoot(dt, mPlayer2, player2Bullets);
	}*/


	switch (buffer[0])
	{
	case Server_Message::State:
	{
		int readIndex = 1;
		float bufferX = 0, bufferY = 0, rot = 0;

		memcpy(&bufferX, &buffer[readIndex], sizeof(float));
		readIndex += sizeof(float);

		memcpy(&bufferY, &buffer[readIndex], sizeof(float));
		readIndex += sizeof(float);
		
		memcpy(&rot, &buffer[readIndex], sizeof(float));
		readIndex += sizeof(float);

		mPlayer2.spr.setPosition(bufferX, bufferY);
		mPlayer2.spr.setRotation(rot);

	}
	break;
	case Server_Message::AsteroidData:
	{
			int readIndex = 1;
			for (auto& ast : asteroids)
			{
				// Store temp asteroid data
				float x = 0; float y = 0; float rot = 0;
				bool destroyed = false;

				// Get the flag for if the asteroid is destroyed on the server
				memcpy(&destroyed, &buffer[readIndex], sizeof(destroyed));
				readIndex += sizeof(&ast->isDestroyed);
				// Get the asteroids velocity from the server
				memcpy(&x, &buffer[readIndex], sizeof(float));
				readIndex += sizeof(&ast->velocity);

				memcpy(&y, &buffer[readIndex], sizeof(float));
				readIndex += sizeof(&ast->velocity);

				memcpy(&rot, &buffer[readIndex], sizeof(bool));
				readIndex += sizeof(&ast->velocity);

				ast->isDestroyed = destroyed;
				ast->spr.setPosition({ x,y });
				ast->spr.setRotation(rot);
			}
	}
	break;
	default: break;
	}

}

void Game::DeserialiseAsteroidData(std::string& astData)
{
	std::string test = astData;

	float x, y, r;
	int elementPos;
	bool destroyed = false;
	int pos = 0;
	std::string result;
	bool finished = false;
	if (strstr(astData.c_str(), "Asteroids"))
	{
		pos = astData.find_first_of('[');
		astData.erase(astData.begin(), astData.begin() + pos + 1);
		//pos++;

		
		while ( !astData.empty() && astData.front() != ']')
		{
			if (astData[pos] != ',')
			{
				// Find element position
				elementPos = FindNextValue(astData, 1, '/');
				
				// Find destroyed state
				destroyed = FindNextValue(astData, 1, '/');
				// Find all positional information
				x = FindNextValue(astData, 7, '.');
				y = FindNextValue(astData, 7, '.');
				r = FindNextValue(astData, 7, '.');

				//if (!(elementPos > asteroids.size()))
				//{
					// Apply updated information to specific asteroid
					asteroids[elementPos]->spr.setPosition(x, y);
					asteroids[elementPos]->spr.setRotation(r);
					asteroids[elementPos]->isDestroyed = destroyed;

					astData.erase(astData.begin());
				//}



			}
			else
				astData.erase(astData.begin());
	
			
		}

	}
}

void Game::DeserialisePlayerData(std::string& pData)
{
	int pos = 0, lives, score = 0;
	
	if (strstr(pData.c_str(), "Info/"))
	{
		pData.erase(pData.begin(), pData.begin() + pos + 5);

		score = FindNextValue(pData, 1, '/');
		lives = FindNextValue(pData, 1, '/');

		mPlayer2.score = score;

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

		if (str.front() == ',')
			str.erase(str.begin());

		while (str[pos] != seperator)
		{
			temp += str[pos++];
		}

		temp += str.substr(pos, pos + charAmount);

		val = std::stof(temp);

		str.erase(str.begin(), str.begin() + charAmount + pos);

		pos = 0;
	}
	else
	{

		if (str.front() == '/')
			str.erase(str.begin());

		while (str[pos] != seperator)
		{
			temp += str[pos++];
		}

		val = std::stof(temp);

		str.erase(str.begin(), str.begin() + charAmount + 1);

		pos = 0;
	}


	return val;
}


std::string Game::SendGameData()
{
	std::string posPacket = isHost ? mPlayer.SerializeData() + SendAsteroidData() + SendPlayerInfoData() : mPlayer.SerializeData() + SendPlayerInfoData();

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
		asteroidsStr += std::to_string(elementPos) + seperator +										  // Element position in vector
			BoolToString(a->isDestroyed) + seperator  +													 //  destroyed state
			std::to_string(x) + seperator + std::to_string(y) + seperator + std::to_string(r) + ',';	//  Positional data

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

void Game::Shoot(const float& dt, Player& originPlayer, std::vector<Bullet*>& bulletPool)
{
	sf::Time now = shootClock.getElapsedTime();
	if ((now - lastFired) >= originPlayer.GetShootDelay())
	{
		lastFired = now;
		auto bullet = std::find_if(bulletPool.begin(), bulletPool.end(), [](Bullet* b) {return b->bInPool; });
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

bool Game::GameOver()
{
	// Handle game over logic
	if (mPlayer.score == 100 || mPlayer2.lives == 0)
	{
		displayP1WinTxt = true;
		displayP2WinTxt = false;

		return true;
	}

	if (mPlayer2.score == 100 || mPlayer.lives == 0)
	{
		displayP2WinTxt = true;
		displayP1WinTxt = false;

		return true;
	}

	return false;
}

char* Game::CreatePlayerPosPacket()
{
		const int bufferSize = 1024;
		char buffer[bufferSize];
		buffer[0] = Client_Message::Input;
		uint32_t bytesWritten = 1;
		float x = mPlayer.GetSprite().getPosition().x;
		float y = mPlayer.GetSprite().getPosition().y;
		float r = mPlayer.GetSprite().getRotation();
		
		memcpy(&buffer[bytesWritten], &x, sizeof(float));
		bytesWritten += sizeof(float);

		memcpy(&buffer[bytesWritten], &y, sizeof(float));
		bytesWritten += sizeof(float);

		memcpy(&buffer[bytesWritten], &r, sizeof(float));
		bytesWritten += sizeof(float);
		
		return buffer;
}

char* Game::CreateAsteroidPacket()
{
	// Packet Structure
	// [DataType/isDestroyed/Velocity]
	const int bufferSize = 1024;
	char buffer[bufferSize];
	buffer[0] = Server_Message::AsteroidData;
 	uint32_t bytesWritten = 1;

	for (auto& ast : asteroids)
	{
		float rot = ast->spr.getRotation();
		memcpy(&buffer[bytesWritten], &ast->isDestroyed, sizeof(bool));
		bytesWritten += sizeof(bool);

		memcpy(&buffer[bytesWritten], &ast->spr.getPosition().x, sizeof(&ast->spr.getPosition().x));
		bytesWritten += sizeof(ast->spr.getPosition().x);

		memcpy(&buffer[bytesWritten], &ast->spr.getPosition().y, sizeof(&ast->spr.getPosition().y));
		bytesWritten += sizeof(ast->spr.getPosition().y);

		memcpy(&buffer[bytesWritten], &rot, sizeof(&rot));
		bytesWritten += sizeof(rot);

	}

	return buffer;
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
				a->spr.setPosition(-1380, 800);
				a->velocity = sf::Vector2f(0, 0);
				a->isDestroyed = true;

				mPlayer.AddScore(10);
				b->bInPool = true;
				b->velocity = { 0, 0};
				b->spr.setPosition(1380, 800); // Object pool position

			}
		}

		for (auto& b : player2Bullets)
		{
			if (a->spr.getGlobalBounds().intersects(b->spr.getGlobalBounds()))
			{
				// Destroy asteroid
				a->spr.setPosition(-1380, 800);
				a->velocity = sf::Vector2f(0, 0);
				a->isDestroyed = true;

				b->bInPool = true;
				b->velocity = { 0, 0 };
				b->spr.setPosition(1380, 800); // Object pool position

			}
		}

		
	}
}

