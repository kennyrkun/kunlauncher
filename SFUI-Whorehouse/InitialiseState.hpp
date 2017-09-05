#ifndef INITALISE_STATE_HPP
#define INITALISE_STATE_HPP

#include <SFML\Graphics.hpp>
#include "AppState.hpp"
#include "Scrollbar.hpp"

#include <thread>
#include <vector>

class Link;
class Item;

struct LaunchOptions
{
#ifdef _DEBUG
	bool updateItemsOnStart = true;
	bool updateLauncherOnStart = true;
#else
	bool updateItemsOnStart = true;
	bool updateLauncherOnStart = true;
#endif
	int width = 525;
};


class InitialiseState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

	static InitialiseState* Instance()
	{
		return &IntialiseState_dontfuckwithme;
	}

protected:
	InitialiseState() { }

private:
	static InitialiseState IntialiseState_dontfuckwithme;
	AppEngine* app;

	sf::View *mainView;
	sf::View *cardScroller;
	Scrollbar scrollbar;

	LaunchOptions settings;
	std::vector<std::thread> threads;
	std::vector<Item*> items;
	std::vector<Link*> links;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	sf::Font font;
	sf::Text initalisingText;
	sf::Text currentLauncherTask;
	sf::Text currentLauncherSubtask;

	void initialisise();
	void loadApps();

	bool checkForLauncherUpdates();
	std::string updateLauncher();

	void updateScrollThumb();

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Text &object);

	void setTaskText(std::string text);
	void setTaskSubtext(std::string text);
};

#endif // !INITALISE_STATE_HPP