#ifndef PROGRAM_ENGINE_HPP
#define PROGRAM_ENGINE_HPP

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <thread>

class AppState;

//TODO: don't set these if they've been overriden via command line arugements
struct AppSettings
{
	//	bool offline = false

	bool updateOnStart = false;
	bool logDownloads = true;
	bool newsEnabled = true;
	bool SFUIDebug = false;
	bool allowStatTracking = false;
	bool useAnimations = false;
	int animationScale = 1;

	std::string selectedTheme = "dark";

	struct Window
	{
		bool verticalSync = true;
		//		bool useSFUIDecorations = false;
		int width = 525;
		int	height = 375;
	} window;

	struct Apps
	{
		bool updateStoreOnStart = false;
		bool autoUpdate = false;
		bool checkForUpdates = false;
		//  bool showInstalledAppsInAllAppsList
	} apps;
};

class AppEngine
{
public:
	void Init(std::string title, AppSettings settings_);
	void Cleanup();

	// THE ACTIVE STATE IS NOT SWITCHED UNTIL HandleEvents() HAS RETURNED
	void ChangeState(AppState* state);
	// THE ACTIVE STATE IS NOT SWITCHED UNTIL HandleEvents() HAS RETURNED
	void PushState(AppState* state);
	// THE ACTIVE STATE IS NOT SWITCHED UNTIL HandleEvents() HAS RETURNED
	void PopState();
	AppState* GetCurrentState() { return states.back(); };

	void HandleEvents();
	void Update();
	void Draw();

	bool Running() { return running; }
	// THE ACTIVE STATE IS NOT SWITCHED UNTIL HandleEvents() HAS RETURNED
	void Quit() { running = false; }

	void UpdateViewSize(const sf::Vector2f& size);

	bool multithreaded_process_running;
	bool multithreaded_process_finished;
	void ShowMultiThreadedIndicator();
	void SetMultiThreadedIndicatorPosition(const sf::Vector2f& pos);
	void LoadMultiThreadedIcon(std::string iconPath);
	std::thread *multithread;
	/// should be deleted by Update method of State in which it is used.

	sf::RenderWindow* window;
	AppSettings settings;

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

	sf::CircleShape multithreaded_process_indicator;
	sf::Texture multithreaded_process_indicator_tex;
};

#endif // !PROGRAM_ENGINE_HPP
