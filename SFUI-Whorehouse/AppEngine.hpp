#ifndef PROGRAM_ENGINE_HPP
#define PROGRAM_ENGINE_HPP

#include <SFML\Graphics.hpp>
#include <vector>
#include <string>

class AppState;

//TODO: don't set these if they've been overriden via command line arugements
struct AppSettings
{
#ifdef _DEBUG
	bool updateLauncherOnStart = false;
	bool checkForNewItemsOnStart = false;
	bool experimentalThemes = true;
#else
	bool updateLauncherOnStart = true;
	bool checkForNewItemsOnStart = true;
	bool experimentalThemes = false;
#endif
	bool fullscreen = false;
	bool verticalSync = true;

	int width = 525;
	int height = 375;

	std::string theme = "THEMEFUCKER";
};

class AppEngine
{
public:
	void Init(std::string title, AppSettings settings_);
	void Cleanup();

	void ChangeState(AppState* state);
	void PushState(AppState* state);
	void PopState();

	void HandleEvents();
	void Update();
	void Draw();

	bool Running() { return m_running; }
	void Quit() { m_running = false; }

	sf::RenderWindow* window;
	AppSettings settings;
	bool developerModeActive = false;
	std::string title;

private:
	// the stack of states
	std::vector<AppState*> states;

	bool m_running;
};

#endif // !PROGRAM_ENGINE_HPP
