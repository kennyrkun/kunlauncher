#ifndef ALL_APPS_LIST_STATE_HPP
#define ALL_APPS_LIST_STATE_HPP

#include "AppState.hpp"

#include "VerticalScrollbar.hpp"

#include <SFML/Graphics.hpp>

#include <future>
#include <vector>
#include <iostream>

class StoreApp;

class AllAppsListState : public AppState
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

	sf::View *mainView;
	sf::View *viewScroller;
	VerticalScrollbar scrollbar;

	std::vector<StoreApp*> apps;

	void loadApps(bool &finisehdIndicator);

	// TODO: viewableArea class
	float scrollerTopPosition;
	float scrollerBottomPosition;
	float scrollerMinPosition;
	float scrollerMaxPosition;
	void updateScrollThumbSize();
	void testScrollBounds();
	void updateScrollLimits();

	bool mouseIsOver(const sf::Shape &object);
	bool mouseIsOver(const sf::Shape &object, const sf::View* view);
	bool mouseIsOver(const sf::Text &object);
	bool mouseIsOver(const sf::Text &object, const sf::View* view);
};

#endif // !ALL_APPS_LIST_STATE_HPP