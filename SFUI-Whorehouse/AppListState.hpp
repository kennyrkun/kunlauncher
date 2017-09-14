#ifndef APPLIST_STATE_HPP
#define APPLIST_STATE_HPP

#include <SFML\Graphics.hpp>
#include "AppState.hpp"
#include "Scrollbar.hpp"

#include <thread>
#include <vector>

class Link;
class Item;

class AppListState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

	static AppListState* Instance() 
	{
		return &AppListState_dontfuckwithme;
	}

protected:
	AppListState() { }

private:
	static AppListState AppListState_dontfuckwithme;
	AppEngine* app;

	sf::View *mainView;
	sf::View *cardScroller;
	Scrollbar scrollbar;

	std::vector<std::thread> threads;
	std::vector<Item*> items;
	std::vector<Link*> links;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;

	void loadApps();

	void updateScrollThumb();

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Shape &object, sf::View* view);
	bool mouseIsOver(sf::Text &object);
};

#endif // !APPLIST_STATE_HPP