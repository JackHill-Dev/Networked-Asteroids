#pragma once
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <functional >
enum Move
{
	Hold, Forward
};

enum Rotate
{
	Still, Left, Right
};

class Player
{
public:
	Player();
	void Update(const float& deltaTime);
	void Update(const float& deltaTime, sf::Vector2f& vel);

	void AddScore(const int& s);
	void RemoveLife() { --lives; };
	void AssignID(const int& i) { id = i; };
	sf::Time& GetShootDelay() { return fireDelay; }
	std::string SerializeData();
	void DesrializeData(std::string& data);
	const sf::FloatRect& GetBounds() const;
	 sf::Sprite& GetSprite();
	 sf::Vector2f& GetVelocity() { return velocity; }
	 int score = 0;
	 int lives = 3;
	float ToRadians(float x)
	{
		return	x * (3.14 / 180);
	}
	bool bHasCollided = false;

	Move move = Hold;
	Rotate rotate = Still;
	void SetSpriteColor(const sf::Color& col) { spr.setColor(col); }
	float& FindNextValue(std::string& str);
	void Move(const float& deltaTime);

	sf::Sprite spr;
	sf::Texture texture;
	sf::Vector2f velocity = {0,0};
	sf::Time fireDelay = sf::seconds(0.25);
	int id = -1;
	float thrustSpeed = 0.2f;
	float rotationSpeed = 100.f;
};

