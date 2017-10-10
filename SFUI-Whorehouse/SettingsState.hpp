#ifndef SETTINGS_STATE_HPP
#define SETTINGS_STATE_HPP

#include "AppState.hpp"
#include "Scrollbar.hpp"

#include <SFML\Graphics.hpp>
#include <thread>

class LauncherUpdater;
class ProgressBar;

class SettingsState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents(sf::Event& event);
	void Update();
	void Draw();

	static SettingsState* Instance()
	{
		return &IntialiseState_dontfuckwithme;
	}

protected:
	SettingsState() { }

private:
	static SettingsState IntialiseState_dontfuckwithme;
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

#endif // !SETTINGS_STATE_HPP
