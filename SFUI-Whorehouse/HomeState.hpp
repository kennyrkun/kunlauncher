#ifndef HOME_STATE_HPP
#define HOME_STATE_HPP

#include <SFML\Graphics.hpp>
#include "AppState.hpp"

#include <vector>

class HomeState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

	static HomeState* Instance()
	{
		return &IntialiseState_dontfuckwithme;
	}

protected:
	HomeState() { }

private:
	static HomeState IntialiseState_dontfuckwithme;
	AppEngine* app;

	sf::Font* font;
	sf::Text homeText;
};

#endif // !HOME_STATE_HPP