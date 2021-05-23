#include "Player.h"
#include <SFML/Window/Keyboard.hpp>
#include <iostream>

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
	// Send player vel/pos data to server here???
}

void Player::Update(const float& deltaTime, sf::Vector2f& vel)
{
	spr.move(vel);
	
	
}

void Player::AddScore(const int& s)
{
	score += s;
}



void Player::Move(const float& deltaTime) //,sf::Keyboard::key& kwy)
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

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		velocity.x = 0;
		velocity.y = 0;
	}
	

}

std::string Player::SerializeData()
{

	float tempX = spr.getPosition().x;
	float tempY = spr.getPosition().y;

	std::string playerData = "Pos/" + std::to_string(tempX) + "/" + std::to_string(tempY) + "/" + std::to_string(spr.getRotation()) + "/\0";

	return playerData;
}

float& Player::FindNextValue(std::string& str)
{
	std::string temp;
	int pos = 0;
	float val = 0;

	if(str.front() == '/')
		str.erase(str.begin());

	while (str[pos] != '.')
	{
		temp += str[pos++];
	}

	temp += str.substr(pos, pos + 7);

	val = std::stof(temp);

	str.erase(str.begin(), str.begin() + pos + 7);

	return val;
}

void Player::DesrializeData(std::string& data)
{
	float x, y, r;

	size_t pos = 0;
	std::string result;
	if (strstr(data.c_str(), "Pos"))
	{
		pos = data.find_first_of('/');
		pos++;
		data.erase(data.begin(), data.begin() + pos);

		x = FindNextValue(data);
		y = FindNextValue(data);
		r = FindNextValue(data);

		spr.setPosition(x, y);
		spr.setRotation(r);
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
