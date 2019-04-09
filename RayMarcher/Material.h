#pragma once
#include "Includes.h"
class Material
{
public:
	sf::Color color = sf::Color::Red;
	Material();
	Material(sf::Color color);
	~Material();
};

