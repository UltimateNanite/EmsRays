#include "stdafx.h"
#include "Light.h"


Light::Light()
{
}


Light::~Light()
{
}
Light::Light(int type, sf::Vector3f pos, sf::Vector3f direction) : type(type), pos(pos), direction(direction) {

}