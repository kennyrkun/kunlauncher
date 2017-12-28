#ifndef APPLIST_STATE_HPP
#define APPLIST_STATE_HPP

#include "AppState.hpp"

#include <SFML/Graphics.hpp>
#include <SFUI/Scrollbar.hpp>

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
	SFUI::Scrollbar scrollbar;

	sf::RectangleShape tracker1;
	sf::RectangleShape tracker2;
	sf::RectangleShape tracker3;

	std::vector<std::thread> threads;
	std::vector<Item*> items;
	std::vector<Link*> links;

	std::thread *helperThread;
	bool helperRunning = false;
	bool helperDone = false;
	bool loadingApps = false;

	void loadApps();

	void updateScrollThumbSize();
	// TODO: viewable arae class
	float scrollerTopPosition;
	float scrollerBottomPosition;
	float scrollerMinPosition;
	float scrollerMaxPosition;
	void updateScrollLimits();

	bool mouseIsOver(const sf::Shape &object);
	bool mouseIsOver(const sf::Shape &object, const sf::View* view);
	bool mouseIsOver(const sf::Text &object);
};

#endif // !APPLIST_STATE_HPP