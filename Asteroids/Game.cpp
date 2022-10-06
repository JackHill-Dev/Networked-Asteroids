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
	else
	{
		for (auto& a : asteroids)
		{
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
	case Client_Message::PlayerData:
	case Server_Message::State:
	{
		int readIndex = 1;
		float bufferX = 0, bufferY = 0, rot = 0;
		PlayerDataPacket playerData;


		memcpy(&bufferX, &buffer[readIndex], sizeof(PlayerDataPacket));
		readIndex += sizeof(PlayerDataPacket);

		mPlayer2.spr.setPosition(playerData.Position);
		mPlayer2.spr.setRotation(playerData.Rotation);

	}
	break;
	case Server_Message::AsteroidData:
	{
		// Packet Structure
		// [DataType/AsteroidDataPacket]
		if (!isHost)
		{
			int readIndex = 1;

			for (size_t i = 0; i < asteroids.size(); i++)
			{
				AsteroidDataPacket astData;
				memcpy(&astData, &buffer[readIndex], sizeof(AsteroidDataPacket));
				asteroids[astData.id]->isDestroyed = astData.Destroyed;
				asteroids[astData.id]->spr.setPosition(astData.Position);
				asteroids[astData.id]->spr.setRotation(astData.Rotation);
				asteroids[astData.id]->velocity = astData.Velocity;

				readIndex += sizeof(AsteroidDataPacket);
			}
		}

	}
	break;
	default: break;
	}

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

char* Game::CreatePlayerPosPacket(bool isHost)
{
		const int bufferSize = 1024;
		char buffer[bufferSize];
		buffer[0] = isHost ? Server_Message::State : Client_Message::PlayerData;
		uint32_t bytesWritten = 1;
		PlayerDataPacket playerData =
		{
			.Rotation = mPlayer.GetSprite().getRotation(),
			.Position = mPlayer.GetSprite().getPosition()
		};
		
		memcpy(&buffer[bytesWritten], &playerData, sizeof(PlayerDataPacket));
		bytesWritten += sizeof(PlayerDataPacket);
		
		return buffer;
}

char* Game::CreateAsteroidPacket()
{
	// Packet Structure
	// [DataType/AsteroidDataPacket]
	const int bufferSize = 1024;
	char buffer[bufferSize];
	buffer[0] = Server_Message::AsteroidData;
 	uint32_t bytesWritten = 1;

	for (int i = 0; i < asteroids.size(); ++i)
	{
		AsteroidDataPacket astData =
		{
			.id = i,
			.Destroyed = asteroids[i]->isDestroyed,
			.Rotation = asteroids[i]->spr.getRotation(),
			.Position = asteroids[i]->spr.getPosition(),
			.Velocity = asteroids[i]->velocity
		};

		memcpy(&buffer[bytesWritten], &astData, sizeof(AsteroidDataPacket));
		bytesWritten += sizeof(AsteroidDataPacket);
	}

	return buffer;
}

char* Game::CreateAsteroidPacket(int objectIndex)
{
	const int bufferSize = 1024;
	char buffer[bufferSize];
	buffer[0] = Server_Message::AsteroidData;
	uint32_t bytesWritten = 1;

	Asteroid& ast = *asteroids[objectIndex];
	float rot = ast.spr.getRotation();


	AsteroidDataPacket astData =
	{
		.id = objectIndex,
		.Destroyed = ast.isDestroyed,
		.Rotation = ast.spr.getRotation(),
		.Position = ast.spr.getPosition(),
		.Velocity = ast.velocity
	};

	memcpy(&buffer[bytesWritten], &astData, sizeof(AsteroidDataPacket));
	bytesWritten += sizeof(AsteroidDataPacket);

	return buffer;
}

char* Game::CreatePlayerInfoPacket()
{
	const int bufferSize = 1024;
	char buffer[bufferSize];
	buffer[0] = Server_Message::AsteroidData;
	uint32_t bytesWritten = 1;

	PlayerInfoPacket playerInfo =
	{
		.Score = mPlayer.score,
		.Lives = mPlayer.lives
	};

	memcpy(&buffer[bytesWritten], &playerInfo, sizeof(PlayerInfoPacket));
	bytesWritten += sizeof(PlayerInfoPacket);

	return buffer;
}

int Game::RandomNumberGenerator(int min, int max)
{
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(random_number_engine);
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

