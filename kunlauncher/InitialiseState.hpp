#ifndef INITALISE_STATE_HPP
#define INITALISE_STATE_HPP

#include "AppState.hpp"

#include <SFML/Graphics.hpp>
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

	void HandleEvents();
	void Update();
	void Draw();

private:
	AppEngine* app;
	// FIXME: shouldn't need to be a pointer

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	bool updatIsAvailable = false;
	bool isReady = false;
	bool restartNow = false;

	void initialise();

	int validateFileStructure();
	int validateResourceFiles();

	int getThemeConfiguration();

	sf::Font font;
	sf::Text initialiseText;
	sf::RectangleShape thingthatdoesntripoffvisualstudio;
	ProgressBar* progressBar;

	sf::Text currentLauncherTask;
	void setTaskText(std::string text);

	sf::Clock initTime;
};

#endif // !INITALISE_STATE_HPP
