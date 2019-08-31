#ifndef ANIMATION_MANAGER_HPP
#define ANIMATION_MANAGER_HPP

#include "../App.hpp"

#include <SFML/Graphics.hpp>

#include <functional>
#include <iostream>
#include <vector>

class AppEngine;

enum class EaseType
{
	BackEaseIn,
	BackEaseOut,
	BackEaseInOut,
	BounceEaseIn,
	BounceEaseOut,
	BounceEaseInOut,
	CircEaseIn,
	CircEaseOut,
	CircEaseInOut,
	CubicEaseIn,
	CubicEaseOut,
	CubicEaseInOut,
	ElasticEaseIn,
	ElasticEaseOut,
	ElasticEaseInOut,
	ExpoEaseIn,
	ExpoEaseOut,
	ExpoEaseInOut,
	LinearEaseNone,
	LinearEaseIn,
	LinearEaseOut,
	LinearEaseInOut,
	QuartEaseIn,
	QuartEaseOut,
	QuartEaseInOut,
	QuadEaseIn,
	QuadEaseOut,
	QuadEaseInOut,
	QuintEaseIn,
	QuintEaseOut,
	QuintEaseInOut
};

class AnimatedTask
{
public:
	virtual ~AnimatedTask() = 0;

	bool constant = false;

	int animationID = 0;

	// TODO: find a better way to do this
	virtual size_t getCurrent_st();
	virtual float getCurrent_f();
	virtual sf::Vector2f getCurrent_v2f();

	virtual bool pastTime() = 0;

	virtual void Update() = 0;

private:
	std::function<float(float t, float b, float c, float d)> easeFunction;
};

class AnimatedNumber : public AnimatedTask
{
public:
	AnimatedNumber(size_t original, size_t& target, std::function<float(float, float, float, float)> easeFunction, int duration, bool constant, int ID);
	~AnimatedNumber();

	int animationID;

	int duration; // miliseconds
	sf::Clock tick;

	// get the current value of the animated number
	size_t getCurrent() { return current; };

	bool pastTime();
	bool constant = false;

	size_t& target;
	size_t original;

	void Update();

private:
	size_t changeInNumber;
	size_t current;

	std::function<float(float t, float b, float c, float d)> easeFunction;
};

class AnimatedRotation : public AnimatedTask
{
public:
	AnimatedRotation(sf::Transformable &shape, float targetRotation, std::function<float(float, float, float, float)> easeFunction, int duration, bool constant, int ID);
	~AnimatedRotation();

	int animationID;

	int duration; // miliseconds
	sf::Clock tick;

	float getCurrent() { return current; };

	bool pastTime();
	bool constant = false;

	sf::Transformable& shape;

	float targetRotation;
	float originalRotation;

	void Update();

private:
	float changeInRotation;
	float current;

	std::function<float(float t, float b, float c, float d)> easeFunction;
};

class AnimatedTranslation : public AnimatedTask
{
public:
	AnimatedTranslation(sf::Transformable &shape, sf::Vector2f targetPosition, std::function<float(float, float, float, float)> easeFunction, int duration, bool constant, int ID);
	~AnimatedTranslation();

	int animationID;

	int duration; // miliseconds
	sf::Clock tick;

	sf::Vector2f getCurrent() { return current; };

	bool pastTime();
	bool constant = false;

	sf::Transformable& shape;

	sf::Vector2f targetPosition;
	sf::Vector2f originalPosition;

	void Update();

private:
	sf::Vector2f changeInPosition;
	sf::Vector2f current;

	std::function<float(float t, float b, float c, float d)> easeFunction;
};

class AnimatedRectangleSize : public AnimatedTask
{
public:
	AnimatedRectangleSize(sf::RectangleShape &shape, sf::Vector2f targetSize, std::function<float(float, float, float, float)> easeFunction, int duration, bool constant, int ID);
	~AnimatedRectangleSize();

	int animationID;

	int duration; // miliseconds
	sf::Clock tick;

	sf::Vector2f getCurrent() { return current; };

	bool pastTime();
	bool constant = false;

	sf::RectangleShape& shape;

	sf::Vector2f targetSize;
	sf::Vector2f originalSize;

	void Update();

private:
	sf::Vector2f changeInSize;
	sf::Vector2f current;

	std::function<float(float t, float b, float c, float d)> easeFunction;
};

class AnimatedAppTranslation : public AnimatedTask
{
public:
	AnimatedAppTranslation(App* app, sf::Vector2f targetPosition, std::function<float(float, float, float, float)> easeFunction, int duration, bool constant, int ID);
	~AnimatedAppTranslation();

	int animationID;

	int duration; // miliseconds
	sf::Clock tick;

	sf::Vector2f getCurrent() { return current; };

	bool pastTime();
	bool constant = false;

	App* app;

	sf::Vector2f targetPosition;
	sf::Vector2f originalPosition;

	void Update();

private:
	sf::Vector2f changeInPosition;
	sf::Vector2f current;

	std::function<float(float t, float b, float c, float d)> easeFunction;
};

class PhysicalAnimator
{
public:
	PhysicalAnimator(AppEngine* app);
	~PhysicalAnimator();

	int addTranslationTask(sf::Transformable& shape, sf::Vector2f destination, EaseType ease, int duration, bool constant = false);
	int addRectangleSizeTask(sf::RectangleShape& shape, sf::Vector2f size, EaseType ease, int duration, bool constant = false);
	int addAppTranslationTask(App* app, sf::Vector2f destination, EaseType ease, int duration, bool constant = false);
	int addRotationTask(sf::Transformable& shape, float& targetRotation, EaseType ease, int duration, bool constant = false);

	void updateTaskTarget(size_t taskID, size_t newTarget);
	void updateTaskTarget(size_t taskID, sf::Vector2f newTarget);

	void clearTasks();

	void Update();

	std::vector<AnimatedTask*> tasks;

private:
	AppEngine* app;

	size_t totalAnimations = 0;
};

#endif // !ANIMATION_MANAGER_HPP
