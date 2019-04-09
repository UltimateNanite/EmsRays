#include "stdafx.h"
#include "RM_Object.h"


RM_Object::RM_Object()
{
}


RM_Object::~RM_Object()
{
}

float RM_Object::length(sf::Vector3f vec) {
	return sqrt(vec.x*vec.x + vec.y*vec.y + vec.z*vec.z);
}

RM_Object::RM_Object(sf::Vector3f position, int type, sf::Vector3f size) : position(position), type(type), size(size) {
}
RM_Object::RM_Object(sf::Vector3f position, int type, sf::Vector3f size, Material mat) : position(position), type(type), size(size), material(mat) {
}