#ifndef INITALISE_STATE_HPP
#define INITALISE_STATE_HPP

#include <SFML\Graphics.hpp>
#include "AppState.hpp"
#include "Scrollbar.hpp"

#include <thread>

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

	LaunchOptions settings;
	bool developerActivated = 0;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;
	bool isReady = false;

	void initialisise();
	bool checkForLauncherUpdates();
	std::string updateLauncher();
	int updateResourceFiles();

	sf::Font font;
	sf::Text homeText;
	sf::Text currentLauncherTask;
	void setTaskText(std::string text);
	sf::Text currentLauncherSubtask;
	void setTaskSubtext(std::string text);
};

#endif // !INITALISE_STATE_HPP