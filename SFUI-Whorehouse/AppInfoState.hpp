#ifndef APP_INFO_STATE
#define APP_INFO_STATE

#include "AppEngine.hpp"
#include "AppState.hpp"

#include <iostream>

//----------------------------------
// This class doesn't have a constructor or deconstructor,
// instead, it has a static member object of itself.
// when initialised via AppEngine::PushState or AppEngine::ChangeState,
// this->Instance is called. This returns the static object of itself which is then initalised,
// and then Init is called.
// Cleanup is called on destrcuction via AppEngine.
//----------------------------------
/*
class AppInfoState : public AppState2
{
public:
	AppInfoState(AppEngine2* app_);
	~AppInfoState();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

private:
	AppEngine2* app;

	sf::Text text;
	sf::Font* font;
	sf::Font font2;
};
*/

class AppInfoState2 : public AppState2
{
public:
//	void Init(AppEngine* app_);
//	void Cleanup();

	AppInfoState2(AppEngine2* app_, int number_);
	~AppInfoState2();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update(); 
	void Draw();

private:
	AppEngine2* app;

	int number = 0;
	sf::Text text;
	sf::Font* font;
	sf::Font font2;
};

#endif // !APP_INFO_STATE
