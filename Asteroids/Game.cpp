#include "Game.h"
#include <SFML/Window/Keyboard.hpp>
std::mt19937 random_number_engine(time(0));
Game::Game()
{
	bullets.reserve(100);
	asteroids.reserve(50);
	// Fill bullets pool
	for (int i = 0; i < 100; ++i)
	{
		
		bullets.push_back(new Bullet());
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

	for (auto a : asteroids)
	{
		delete a;
		a = nullptr;
		
	}
}

void Game::Update(const float& deltaTime)
{
	mPlayer.Update(deltaTime);

	/*std::string playerData = mPlayer.SerializeData();;
	mPlayer.DesrializeData(playerData);*/

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		Shoot(deltaTime);
	}

	for (auto& b : bullets)
	{
		b->spr.move(b->velocity);
		
		DisableBullet(b); // Checks if bullets are out of range and put
	}

	for (auto& a : asteroids)
	{
		a->spr.rotate(50 * deltaTime);
		a->spr.move(a->velocity * deltaTime);
		WrapObject(a->spr);
	}

	UpdateCollisions(deltaTime);
	WrapObject(mPlayer.GetSprite());
	
	if (asteroids.empty())
	{
		GameOver();
	}

}

void Game::Draw(sf::RenderWindow& wnd)
{

	for (auto& b : bullets)
	{
		wnd.draw(b->spr);
	}

	for (auto& a : asteroids)
	{
		wnd.draw(a->spr);
	}

	wnd.draw(mPlayer.GetSprite());

	//wnd.draw(mPlayer2.GetSprite());
}

void Game::Shoot(const float& dt)
{
	sf::Time now = shootClock.getElapsedTime();
	if ((now - lastFired) >= mPlayer.GetShootDelay())
	{
		lastFired = now;
		auto bullet = std::find_if(bullets.begin(), bullets.end(), [](Bullet* b) {return b->bInPool; });
		FireBullet(*bullet, dt);

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

void Game::FireBullet(Bullet* bullet, const float& deltatime)
{

	bullet->spr.setOrigin(4, 4);
	bullet->spr.setPosition(mPlayer.GetSprite().getPosition().x, mPlayer.GetSprite().getPosition().y);
	bullet->spr.setRotation(mPlayer.GetSprite().getRotation());

	bullet->velocity.x = sin(ToRadians(bullet->spr.getRotation())) * 450.f * deltatime;
	bullet->velocity.y = -cos(ToRadians(bullet->spr.getRotation())) * 450.f * deltatime;

	bullet->bInPool = false;
	

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

				b->bInPool = true;
				b->velocity = { 0, 0};
				b->spr.setPosition(1380, 800); // Object pool position

			}
		}

		// Check player collision with asteroid
		if (a->spr.getGlobalBounds().intersects(mPlayer.GetSprite().getGlobalBounds()))
		{
			// Lose life
			// Respawn at center of screen in an invulnerable state for a second
			
		}
	}
}

