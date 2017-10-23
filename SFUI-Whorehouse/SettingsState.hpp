#ifndef SETTINGS_STATE_HPP
#define SETTINGS_STATE_HPP

#include "AppState.hpp"

#include <SFML\Graphics.hpp>
#include <SFUI\IconButton.hpp>

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

	sf::Color bgColor; // and all the code that makes it rainbowy, thanks Lukas. <https://stackoverflow.com/questions/11458552/cycle-r-g-b-vales-as-hue>

	int dr = 0;
	int dg = 0;
	int db = 0;

	sf::Uint8 r = 255, g = 0, b = 0;

	SFUI::IconButton* button;
};

#endif // !SETTINGS_STATE_HPP
