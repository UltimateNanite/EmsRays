#pragma once
#include "Includes.h"
#include "Material.h"
class RM_Object
{
public:
	sf::Vector3f position;
	sf::Vector3f size;
	int type;
	class Type {
	public:
		static const int SPHERE = 1;
		static const int CUBOID = 2;
	};

	Material material;
	static float length(sf::Vector3f vec);
	RM_Object();
	RM_Object(sf::Vector3f position, int type, sf::Vector3f size);
	RM_Object(sf::Vector3f position, int type, sf::Vector3f size, Material mat);
	~RM_Object();
	//float GetSignedDistance(sf::Vector3f pos);
};

