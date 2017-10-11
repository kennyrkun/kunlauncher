#ifndef INITALISE_STATE_HPP
#define INITALISE_STATE_HPP

#include "AppState.hpp"

#include <SFML\Graphics.hpp>
#include <SFUI\Scrollbar.hpp>

#include <thread>

class LauncherUpdater;
class ProgressBar;

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
	// FIXME: shouldn't need to be a pointer
	LauncherUpdater *updater;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	bool updatIsAvailable = false;
	bool isReady = false;
	bool restartNow = false;

	void initialisise();

	int validateFileStructure();
	int updateFileStructure();

	int validateResourceFiles();
	int getResourceFiles();

	sf::Font font;
	sf::Text initialiseText;
	sf::RectangleShape thingthatdoesntripoffvisualstudio;
	ProgressBar* progressBar;

	sf::Text currentLauncherTask;
	void setTaskText(std::string text);
};

#endif // !INITALISE_STATE_HPP
