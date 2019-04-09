#pragma once
#include "Includes.h"
class Light
{
public:
	int type;
	sf::Vector3f pos;
	sf::Vector3f direction;
	Light(int type, sf::Vector3f pos, sf::Vector3f direction);
	Light();
	~Light();
};

