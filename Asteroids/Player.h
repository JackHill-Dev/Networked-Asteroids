#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>


class Player
{
public:
	Player();
	void Update(const float& deltaTime);
	void Shoot(const float& deltaTime);
	sf::Time& GetShootDelay() { return fireDelay; }

	const sf::FloatRect& GetBounds() const;
	 sf::Sprite& GetSprite();

	float ToRadians(float x)
	{
		return	x * (3.14 / 180);
	}

private:
	void Move(const float& deltaTime);

	sf::Sprite spr;
	sf::Texture texture;
	sf::Vector2f velocity;
	sf::Time fireDelay = sf::seconds(0.25);
	float thrustSpeed = 0.1f;
	float rotationSpeed = 100.f;
};

