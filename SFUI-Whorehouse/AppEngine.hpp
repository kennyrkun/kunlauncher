#ifndef APP_ENGINE_HPP
#define APP_ENGINE_HPP

#include "animate/AnimationManager.hpp"
#include "Navbar.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <vector>
#include <thread>

class AppState;

// TODO: don't set these if they've been overriden via command line arugements
struct AppSettings
{
//	bool offline = false

	bool updateOnStart = true;
	// bool restartAfterUpdate = false;
	bool debugLogging = false;
	bool logDownloads = true;
	bool SFUIDebug = false;
	bool allowStatTracking = false;
	bool useAnimations = false;
	int animationScale = 1;

	std::string selectedTheme = "dark";

	struct News
	{
		bool enabled = true;
	} news;

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

	// find a way to change the base directory
	// perhaps a portable boolean
	// maybe a string with the base directory that can be changed
	// ./kunlauncher.exe -basedir "./"
	// ./kunlauncher.exe -basedir "C:/Program Files (x86)/KunLauncher"
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

	AppState* GetCurrentState() { return states.back(); };

	void HandleEvents();
	void Update();
	void Draw();

	bool Running() { return running; }

	// THE ACTIVE STATE IS NOT SWITCHED UNTIL HandleEvents() HAS RETURNED
	void Quit();

	void UpdateViewSize(const sf::Vector2f& size);

	bool multithreaded_process_running;
	bool multithreaded_process_finished;
	void ShowMultiThreadedIndicator();
	void SetMultiThreadedIndicatorPosition(const sf::Vector2f& pos);
	void SetMultiThreadedIndicatorIcon(sf::Texture* texture);
	std::thread *multithread;
	/// should be deleted by Update method of State in which it is used.

	const std::string currentDateTime();

	sf::RenderWindow* window;
	AppSettings settings;

	Navbar* navbar;

	PhysicalAnimator am;

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

#endif // !APP_ENGINE_HPP
