#include "Game.h"
#include <SFML/Window/Keyboard.hpp>

Game::Game()
{
	bullets.reserve(100);
	// Fill bullets pool
	for (int i = 0; i < 100; ++i)
	{
		bullets.push_back(Bullet());
	}



	asteroids.push_back(a1);
	asteroids.push_back(a2);
	a1.velocity = { -0.15, -0.1 };
	asteroids[0].spr.setPosition(640, 320);
	asteroids[0].spr.setOrigin(32, 32);
	asteroids[1].spr.setPosition(640, 560);
	asteroids[1].spr.setOrigin(32, 32);

}



void Game::Update(const float& deltaTime)
{
	mPlayer.Update(deltaTime);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		Shoot(deltaTime);
	}

	for (auto& b : bullets)
	{
		b.spr.move(b.velocity);
		
		DisableBullet(b); // Checks if bullets are out of range and put
	}

	for (auto& a : asteroids)
	{
		a.spr.rotate(50 * deltaTime);
		a.spr.move(a.velocity);
		WrapObject(a.spr);
	}

	UpdateCollisions(deltaTime);
	WrapObject(mPlayer.GetSprite());
	

}

void Game::Draw(sf::RenderWindow& wnd)
{
	wnd.draw(mPlayer.GetSprite());

	for (auto& b : bullets)
	{
		wnd.draw(b.spr);
	}

	for (auto& a : asteroids)
	{
		wnd.draw(a.spr);
	}

}

void Game::Shoot(const float& dt)
{
	sf::Time now = shootClock.getElapsedTime();
	if ((now - lastFired) >= mPlayer.GetShootDelay())
	{
		lastFired = now;
		auto bullet = std::find_if(bullets.begin(), bullets.end(), [](Bullet& b) {return b.bInPool; });
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

void Game::DisableBullet(Bullet& bullet)
{
	if ((bullet.spr.getPosition().x > 1280) ||
		(bullet.spr.getPosition().x < 0) ||
		(bullet.spr.getPosition().y > 720) ||
		(bullet.spr.getPosition().y < 0))
	{
		bullet.bInPool = true;
		bullet.velocity = { 0, 0 };
		bullet.spr.setPosition(1380, 800); // Object pool position
	}

}

void Game::FireBullet(Bullet& bullet, const float& deltatime)
{

	bullet.spr.setOrigin(4, 4);
	bullet.spr.setPosition(mPlayer.GetSprite().getPosition().x, mPlayer.GetSprite().getPosition().y);
	bullet.spr.setRotation(mPlayer.GetSprite().getRotation());

	bullet.velocity.x = sin(ToRadians(bullet.spr.getRotation())) * 150.f * deltatime;
	bullet.velocity.y = -cos(ToRadians(bullet.spr.getRotation())) * 150.f * deltatime;

	bullet.bInPool = false;
	

}

void Game::UpdateCollisions(const float& deltaTime)
{
	//// Check bullet collison with asteroids
	for (auto& a : asteroids)
	{
		for (auto& b : bullets)
		{
			if (a.spr.getGlobalBounds().intersects(b.spr.getGlobalBounds()))
			{
				// Destroy asteroid
				asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(), [&a](Asteroid& ast) { return a.spr.getPosition() == ast.spr.getPosition(); }));

				b.bInPool = true;
				b.velocity = { 0, 0};
				b.spr.setPosition(1380, 800); // Object pool position

				//bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&b](Bullet& bull) { return b.spr.getPosition() == b.spr.getPosition(); }));
			}
		}

		// Check player collision with asteroid
		if (a.spr.getGlobalBounds().intersects(mPlayer.GetBounds()))
		{
			// Make ship get pushed back in opposite dir of asteroid

		}
	}
}

