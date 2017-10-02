#ifndef PROGRAM_ENGINE_HPP
#define PROGRAM_ENGINE_HPP

#include <SFML\Graphics.hpp>
#include <vector>
#include <string>

class AppState;

struct AppSettings
{
#ifdef _DEBUG
	bool updateItemsOnStart = false;
	bool updateLauncherOnStart = false;
	bool updateItemsDuringRun = false;
#else
	bool updateItemsOnStart = true;
	bool updateLauncherOnStart = true;
	bool updateItemsDuringRun = true;
#endif
	bool fullscreen = false;
	bool verticalSync = true;

	int width = 525;
	int height = 400;
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
