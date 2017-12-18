#ifndef HOME_STATE_HPP
#define HOME_STATE_HPP

#include "AppState.hpp"

#include <SFML/Graphics.hpp>
#include <SFUI/IconButton.hpp>

#include <thread>
#include <vector>

class Section;

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
		return &HomeState_dontfuckwithme;
	}

protected:
	HomeState() { }

private:
	static HomeState HomeState_dontfuckwithme;
	AppEngine* app;

	std::vector<std::thread> threads;
	std::vector<SFUI::IconButton*> sections;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Shape &object, sf::View* view);
	bool mouseIsOver(sf::Text &object);
};

#endif // !HOME_STATE_HPP