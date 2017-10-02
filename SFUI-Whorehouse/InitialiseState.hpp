#ifndef INITALISE_STATE_HPP
#define INITALISE_STATE_HPP

#include <SFML\Graphics.hpp>
#include "AppState.hpp"
#include "Scrollbar.hpp"

#include <thread>

class ProgressBar;
class Link;
class Item;

class InitialiseState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents(sf::Event& event);
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

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	bool isReady = false;

	void initialisise();

	bool checkForLauncherUpdates();
	bool launcherUpdateAvailabe = false;
	bool restartNow = false;
	std::string updateLauncher();

	int validateFileStructure();
	int updateFileStructure();

	int validateResourceFiles();
	int updateResourceFiles();

	sf::Font font;
	sf::Text initialiseText;
	sf::RectangleShape thingthatdoesntripoffvisualstudio;
	ProgressBar* progressBar;

	sf::Text currentLauncherTask;
	void setTaskText(std::string text);
};

#endif // !INITALISE_STATE_HPP