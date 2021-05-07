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

void Player::Update(const float& deltaTime, sf::Vector2f& vel)
{
	spr.move(vel);
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

std::string Player::SerializeData()
{
	double tempVelX = velocity.x;
	double tempVelY = velocity.y;

	std::string playerData = "Velocity/" + std::to_string(tempVelX) + "," + std::to_string(tempVelY) + "/";
								//"Rotation/" + std::to_string( spr.getRotation()) + "/";*/

	return playerData;
}

void Player::DesrializeData(const std::string& data)
{
	double velX, velY;
	int rot;
	size_t pos = 0;

	data.find("/", pos);
	//++pos;

	std::string result = "";
	while (data[pos] != '/')
	{
		result += data[pos++];
	}
	// TODO: improve this algoritm as there will be a lot more data to handle
	if (result == "Velocity")
	{
		++pos;
		result = data[pos];
		
		std::string value = "";
		while (data[pos] != ',')
			value += data[pos++];

		velX = std::stof(value);

		++pos;

		value = "";
		while (data[pos] != '/')
			value += data[pos++];

		
		velY = std::stof(value);
	}

	velocity.x = velX;
	velocity.y = velY;
}

const sf::FloatRect& Player::GetBounds() const
{
	return spr.getGlobalBounds();
}

sf::Sprite& Player::GetSprite() 
{
	return spr;
}
