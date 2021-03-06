// RayMarcher.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Includes.h"

#include "RM_Object.h"
#include "Light.h"
#include <iostream>

#include "Shader.h"
#include <sstream>
#include <math.h>
#include <SFML/OpenGL.hpp>

std::vector<RM_Object> objects = { 
	//RM_Object({0.f, 0.f, 100.f},	RM_Object::Type::SPHERE, {10.f, 0.f, 0.f}, Material(sf::Color::Cyan)),
	RM_Object({20.f, 0.f, 100.f},	RM_Object::Type::SPHERE, {10.f, 0.f, 0.f}, Material(sf::Color(0,255,255))),
	RM_Object({0.f, -10.f, 100.f},	RM_Object::Type::CUBOID, {200.f, 1.f, 200.f})
};
std::vector<Light> lights = {
	Light(1, {200.f, 200.f, 200.f}, {0.01f, 0.0f, 0.0f}),
//	Light(1, {-200.f, 200.f, 200.f}, {0.01f, 0.0f, 0.0f}),
 };
sf::Vector3f camPos;
sf::Vector3f camRot = { 0.f, 180.f, 180.f };
sf::Vector3f camDir;
float camFOV;
sf::Vector2i resolution = { 848, 480 };
float threshold = 10.f;
void render();
void update();

using namespace sf;
RenderWindow window;

bool running = true;

float mouseX;
float mouseY;

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector3<T>& v)
{
	os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const Vector2<T>& v)
{
	os << "(" << v.x << ", " << v.y << ")";
	return os;
}

template<typename T>
T radians(T x) {
	return 2 * 3.1415 * (x / 360);
}



template<typename T>
Vector3<T> getDirectionVector(Vector3<T> eulerAngles) {
	//Yaw = x
	//Pitch = y
	//Roll = z
	//x = cos(yaw)*cos(pitch)
	//y = sin(yaw)*cos(pitch)
	//z = sin(pitch)

	float yaw = radians(eulerAngles.x);
	float pitch = radians(eulerAngles.y);

	sf::Vector3<T> result;
	result.x = sin(yaw);
	result.y = -sin(pitch);//-(sin(pitch)*cos(yaw));
	result.z = -(cos(pitch)*cos(yaw));
	return result;
}

Font font;
Shader marchShader;
RectangleShape canvas = RectangleShape(Vector2f(resolution));
Image drawCanv;
sf::Time dt;
int main()
{

	font.loadFromFile("consola.ttf");
	drawCanv.create(resolution.x, resolution.y);
	if (!sf::Shader::isAvailable()) {
		window.close();
		std::cout << "Shader support is required for rendering. \nShaders aren't supported on this system.";
		system("pause");
		
		return 0;
	}

	if (!marchShader.loadFromMemory(fragmentShaderSource, Shader::Fragment)) {
		window.close();
		std::cout << "Shader initialisation failed.";
		system("pause");
		return 0;
	}
	std::cout << "Initialisation successful.\n";
	window.create(VideoMode(resolution.x, resolution.y), "RayMarcher");



	
	marchShader.setUniform("resolution", resolution);


	sf::Clock deltaClock;
	
	while (running) {
		update();
		render();
		dt = deltaClock.restart();
	}
    return 0;
}

void update() {
	Event event;
	while (window.pollEvent(event)) {
		if (event.type == Event::Closed) {
			running = false;
			return;
		}
	}
	Vector3f camMoveVec;
	if (window.hasFocus()) {
		sf::Vector3f strafeVector = getDirectionVector(camRot - sf::Vector3f(90.f, 0.f, 0.f));
		strafeVector.y = 0;
		if (Keyboard::isKeyPressed(Keyboard::A))
			camPos -= strafeVector;
		if (Keyboard::isKeyPressed(Keyboard::D))
			camPos += strafeVector;
		if (Keyboard::isKeyPressed(Keyboard::S))
			camPos -= camDir;
		if (Keyboard::isKeyPressed(Keyboard::W))
			camPos += camDir;
		if (Keyboard::isKeyPressed(Keyboard::E))
			camPos.y++;
		if (Keyboard::isKeyPressed(Keyboard::Q))
			camPos.y--;
		if (Mouse::isButtonPressed(Mouse::Button::Left)) {
			camRot.x += (Mouse::getPosition().x - mouseX) / 10.f;
			camRot.y += (Mouse::getPosition().y - mouseY) / 10.f;
		}
	}


	camRot.x = std::fmod(camRot.x, 360);
	camRot.y = std::fmod(camRot.y, 360);
	camRot.z = std::fmod(camRot.z, 360);
	if (camRot.x < 0) camRot.x = 360;
	if (camRot.y < 0) camRot.y = 360;
	if (camRot.z < 0) camRot.z = 360;

	
	/*
	if (Mouse::getPosition(window).x < 0)
		Mouse::setPosition(sf::Vector2i(window.getSize().x, Mouse::getPosition().y), window);
	if (Mouse::getPosition(window).y < 0)
		Mouse::setPosition(sf::Vector2i(Mouse::getPosition().x, window.getSize().y), window);

	if (Mouse::getPosition(window).x > window.getSize().x)
		Mouse::setPosition(sf::Vector2i(0, Mouse::getPosition().y), window);
	if (Mouse::getPosition(window).x > window.getSize().y)
		Mouse::setPosition(sf::Vector2i(Mouse::getPosition().x, 0), window);*/

	mouseX = Mouse::getPosition().x;
	mouseY = Mouse::getPosition().y;

	camDir = getDirectionVector(camRot);


	//Push objects to shader
	marchShader.setUniform("objects_size", (int)objects.size());
	for (int i = 0; i < objects.size(); i++) {
		marchShader.setUniform(("objects[" + std::to_string(i) + "].pos"), objects[i].position);
		marchShader.setUniform(("objects[" + std::to_string(i) + "].type"), objects[i].type);
		marchShader.setUniform(("objects[" + std::to_string(i) + "].size"), objects[i].size);
		marchShader.setUniform(("objects[" + std::to_string(i) + "].mat.color"), sf::Glsl::Vec4(objects[i].material.color));
	}
	marchShader.setUniform("lights_size", (int)lights.size());
	for (int i = 0; i < lights.size(); i++) {
		marchShader.setUniform(("lights[" + std::to_string(i) + "].type"), lights[i].type);
		marchShader.setUniform(("lights[" + std::to_string(i) + "].direction"), lights[i].direction);
		marchShader.setUniform(("lights[" + std::to_string(i) + "].pos"), lights[i].pos);
	}
}


void render() {
	//window.clear(Color::Black);
	canvas.setFillColor(sf::Color::White);
	canvas.setPosition(0, 0);
	marchShader.setUniform("camera_pos", camPos);
	marchShader.setUniform("camera_rot", camDir);
	marchShader.setUniform("camera_fov", camFOV);
	window.draw(canvas, &marchShader);
	
	
	Text debugText;
	

	float SDFresult;
	//glGetUniformV
	debugText.setFont(font);
	debugText.setCharacterSize(20);
	std::stringstream str;
	str << "POS: " << camPos << "\nROT: " << camRot << "\nFRAME (MS): " << dt.asMilliseconds();
	debugText.setString(str.str());
	debugText.setPosition(0, 0);
	debugText.setFillColor(Color::White);
	window.draw(debugText);
	window.display();
}