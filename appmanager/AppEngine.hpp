#ifndef APP_ENGINE_HPP
#define APP_ENGINE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <thread>

class AppState;

struct AppSettings
{
	bool logDownloads = true;
	bool SFUIDebug = false;
	bool allowStatTracking = false;
	bool useAnimations = false;
	int animationScale = 1;

	struct Window
	{
		bool verticalSync = true;
		bool useSFUIDecorations = false;
		int width = 525;
		int	height = 410;
	} window;
};

class AppEngine
{
public:
	void Init(std::string title, AppSettings settings_);
	void Cleanup();

	// THE ACTIVE STATE IS NOT SWITCHED UNTIL THE CALLER FUNCTION HAS RETURNED
	void ChangeState(AppState* state);
	// THE ACTIVE STATE IS NOT SWITCHED UNTIL THE CALLER FUNCTION HAS RETURNED
	void PushState(AppState* state);
	// THE ACTIVE STATE IS NOT SWITCHED UNTIL THE CALLER FUNCTION HAS RETURNED
	void PopState();

	// THE ACTIVE STATE IS NOT SWITCHED UNTIL HandleEvents() HAS RETURNED
	void Quit();

	void HandleEvents();
	void Update();
	void Draw();

	bool isRunning() { return running; }

public: // these are functions specific to this application, or general utility functions. not required for the appengine itself.
	const std::string currentDateTime();

	std::vector<std::string> getDirectories(const std::string& s);

	sf::RenderWindow* window;
	AppSettings settings;

	bool updateAppIndex();

	void drawInformationPanel(std::string text);

	// TODO: this should definitely be improved.
	int appToEdit;

private:
	// the stack of states
	std::vector<AppState*> states;

	enum class EventType
	{
		PushState,
		ChangeState,
		PopState,
		Quit
	};

	std::vector<std::pair<EventType, AppState*>> queuedEvents;

	bool running;

};

#endif // !APP_ENGINE_HPP
