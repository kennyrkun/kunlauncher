#ifndef APPLIST_STATE_HPP
#define APPLIST_STATE_HPP

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

class AppListState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

	static AppListState* Instance() 
	{
		return &AppListState_dontfuckwithme;
	}

protected:
	AppListState() { }

private:
	static AppListState AppListState_dontfuckwithme;
	AppEngine* app;

	LaunchOptions settings;
	sf::View *cardScroller;

	std::vector<Item*> items;
	std::vector<Link*> links;

	Scrollbar scrollbar;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	std::string launcherVersion = "0.10.1";

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

#endif // !APPLIST_STATE_HPP