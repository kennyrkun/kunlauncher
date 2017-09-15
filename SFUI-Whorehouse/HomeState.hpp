#ifndef HOME_STATE_HPP
#define HOME_STATE_HPP

#include <SFML\Graphics.hpp>
#include "AppState.hpp"
#include "Scrollbar.hpp"

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

	sf::View *mainView;
	sf::View *cardScroller;
	Scrollbar scrollbar;

	std::vector<std::thread> threads;
	std::vector<Section*> sections;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	void loadApps();

	void updateScrollThumb();

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Shape &object, sf::View* view);
	bool mouseIsOver(sf::Text &object);
};

#endif // !HOME_STATE_HPP