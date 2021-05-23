#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
std::mt19937 random_number_engine(time(0));
Game::Game()
{
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


	for (int i = 0; i < 10; ++i)
	{
		sf::Vector2f velocity = sf::Vector2f(RandomNumberGenerator(-50, 100), RandomNumberGenerator(-50, 100));
		sf::Vector2f startPos = sf::Vector2f(RandomNumberGenerator(0, 1280), RandomNumberGenerator(0, 720));
		asteroids.push_back(new Asteroid(startPos, velocity));
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

	for (auto& a : asteroids)
	{
		a->spr.rotate(50 * deltaTime);
		a->spr.move(a->velocity * deltaTime);
		WrapObject(a->spr);
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

	if (gData == "Fire")
		Shoot(dt, mPlayer2, player2Bullets);

}

std::string Game::SendGameData()
{
	return mPlayer.SerializeData();
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

