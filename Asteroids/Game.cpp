#include "Game.h"
#include <SFML/Window/Keyboard.hpp>

Game::Game()
{
	asteroidTexture.loadFromFile("asteroid.png");
	asteroids.push_back(sf::Sprite(asteroidTexture));
	asteroids.push_back(sf::Sprite(asteroidTexture));

	asteroids[0].setPosition(640, 320);
	asteroids[0].setOrigin(32, 32);
	asteroids[1].setPosition(640, 560);
	asteroids[1].setOrigin(32, 32);

	

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
	}

	UpdateCollisions(deltaTime);
	WrapPlayer();

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
		wnd.draw(a);
	}

}

void Game::Shoot(const float& dt)
{
	sf::Time now = shootClock.getElapsedTime();
	if ((now - lastFired) >= mPlayer.GetShootDelay())
	{
		lastFired = now;
		bullets.push_back(CreateBullet(mPlayer.GetSprite(), dt));

	}

}

void Game::WrapPlayer()
{
	// Wrap player to opposite side of screen
	// TODO: Remove magic numbers for the screen width and height
	if (mPlayer.GetSprite().getPosition().x > 1280) mPlayer.GetSprite().setPosition(0, mPlayer.GetSprite().getPosition().y);
	if (mPlayer.GetSprite().getPosition().x < 0) mPlayer.GetSprite().setPosition(1280, mPlayer.GetSprite().getPosition().y);
	if (mPlayer.GetSprite().getPosition().y > 720) mPlayer.GetSprite().setPosition(mPlayer.GetSprite().getPosition().x, 0);
	if (mPlayer.GetSprite().getPosition().y < 0) mPlayer.GetSprite().setPosition(mPlayer.GetSprite().getPosition().x, 720);

}

Bullet Game::CreateBullet(const sf::Sprite& player, const float& deltatime)
{
	Bullet bullet;
	bullet.spr.setOrigin(4, 4);
	bullet.spr.setPosition(player.getPosition().x, player.getPosition().y);
	bullet.spr.setRotation(player.getRotation());

	bullet.velocity.x = sin(ToRadians(bullet.spr.getRotation())) * 150.f * deltatime;
	bullet.velocity.y = -cos(ToRadians(bullet.spr.getRotation())) * 150.f * deltatime;


	return bullet;
}

void Game::UpdateCollisions(const float& deltaTime)
{
	//// Check bullet collison with asteroids
	for (auto& a : asteroids)
	{
		for (auto& b : bullets)
		{
			if (a.getGlobalBounds().intersects(b.spr.getGlobalBounds()))
			{
				// Destroy asteroid
				asteroids.erase(std::remove_if(asteroids.begin(), asteroids.end(), [&a](sf::Sprite& s) { return a.getPosition() == s.getPosition(); }));
				bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&b](Bullet& bull) { return b.spr.getPosition() == b.spr.getPosition(); }));
			}
		}
	}
}

