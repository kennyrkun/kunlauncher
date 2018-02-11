#ifndef PROGRAM_ENGINE_HPP
#define PROGRAM_ENGINE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class AppState;

//TODO: don't set these if they've been overriden via command line arugements
struct AppSettings
{
	bool updateLauncherOnStart = false;
	bool checkForNewItemsOnStart = false;
	bool printdownloadprogress = true;
	bool fullscreen = false;
	bool verticalSync = true;

	int width = 525;
	int height = 375;

	std::string selectedTheme = "dark";
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

	bool Running() { return running; }
	void Quit() { running = false; }

	void UpdateViewSize(const sf::Vector2f& size);

	std::string title;
	bool developerModeActive = false;

	sf::RenderWindow* window;
	AppSettings settings;

private:
	// the stack of states
	std::vector<AppState*> states;

	bool running;
};

#endif // !PROGRAM_ENGINE_HPP
