#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>


class Player
{
public:
	Player();
	void Update(const float& deltaTime);
	void Update(const float& deltaTime, sf::Vector2f& vel);
	void Shoot(const float& deltaTime);
	sf::Time& GetShootDelay() { return fireDelay; }
	std::string SerializeData();
	void DesrializeData(std::string& data);
	const sf::FloatRect& GetBounds() const;
	 sf::Sprite& GetSprite();
	 sf::Vector2f& GetVelocity() { return velocity; }
	float ToRadians(float x)
	{
		return	x * (3.14 / 180);
	}
	bool bHasCollided = false;
private:
	void Move(const float& deltaTime);

	sf::Sprite spr;
	sf::Texture texture;
	sf::Vector2f velocity = {0,0};
	sf::Time fireDelay = sf::seconds(0.25);
	float thrustSpeed = 0.1f;
	float rotationSpeed = 100.f;
};

