#ifndef APPLIST_STATE_HPP
#define APPLIST_STATE_HPP

#include "AppState.hpp"

#include "Navbar.hpp"
#include "VerticalScrollbar.hpp"

#include <SFML/Graphics.hpp>
#include <future>
#include <vector>
#include <iostream>

// TODO: separate pages for apps
// 

class MyApp;

class MyAppListState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

private:
	AppEngine* app;

	Navbar* navbar;

	sf::View *mainView;
	sf::View *viewScroller;
	VerticalScrollbar scrollbar;

	// TODO: not a pointer
	std::vector<MyApp*> apps;

	// TODO: viewable arae class
	float scrollerTopPosition;
	float scrollerBottomPosition;
	float scrollerMinPosition;
	float scrollerMaxPosition;
	void updateScrollThumbSize();
	void updateScrollLimits();
	void testScrollBounds();

	void loadApps(bool &finishedIndicator);
	void deleteApp(MyApp* whatApp);

	bool mouseIsOver(const sf::Shape &object);
	bool mouseIsOver(const sf::Shape &object, const sf::View* view);
	bool mouseIsOver(const sf::Text &object);
	bool mouseIsOver(const sf::Text &object, const sf::View* view);

	std::vector<std::string> getDirectories(const std::string& s);
};

#endif // !APPLIST_STATE_HPP