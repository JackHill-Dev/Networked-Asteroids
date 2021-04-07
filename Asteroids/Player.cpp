#include "Player.h"
#include <SFML/Window/Keyboard.hpp>


Player::Player()
{
	texture.loadFromFile("Ship.png");
	spr = sf::Sprite(texture);

	spr.setPosition(640, 480);
	spr.setOrigin(16.f, 16.f);
}

void Player::Update(const float& deltaTime)
{
	Move(deltaTime);
	spr.move(velocity);
}

void Player::Move(const float& deltaTime)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		spr.rotate(-rotationSpeed * deltaTime);
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		spr.rotate(rotationSpeed * deltaTime);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		velocity.x += sin(ToRadians(spr.getRotation())) * thrustSpeed * deltaTime;
		velocity.y += -cos(ToRadians(spr.getRotation())) * thrustSpeed * deltaTime;
	}


}

const sf::FloatRect& Player::GetBounds() const
{
	return spr.getGlobalBounds();
}

sf::Sprite& Player::GetSprite() 
{
	return spr;
}
