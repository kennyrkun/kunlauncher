#ifndef LAUNCHER_HPP
#define LAUNCHER_HPP

#include <SFML\Graphics.hpp>
#include <thread>
#include <vector>

#include "Scrollbar.hpp"

class Link;
class Item;

struct LaunchOptions
{
#ifdef _DEBUG
	bool updateItemsOnStart = false;
	bool updateLauncherOnStart = false;
#else
	bool updateItemsOnStart = true;
	bool updateLauncherOnStart = true;
#endif
	int width = 525;
};

class Launcher
{
public:
	Launcher(LaunchOptions options_);
	~Launcher();

	int Main();

private:
	sf::View *cardScroller;
	sf::RenderWindow *window;
	LaunchOptions settings;

	std::vector<Item*> items;
	std::vector<Link*> links;

	Scrollbar scrollbar;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	std::string launcherVersion = "0.8.8";

	sf::Text initalisingText;
	sf::Text currentLauncherTask;
	sf::Text currentLauncherSubtask;

	void initialisise();
	void loadApps();
	bool checkForLauncherUpdates();

	void updateScrollThumb();

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Text &object);

	void setTaskText(std::string text);
	void setTaskSubtext(std::string text);
};

#endif /* LAUNCHER_HPP */
