#ifndef ALL_APPS_LIST_STATE_HPP
#define ALL_APPS_LIST_STATE_HPP

#include "AppState.hpp"

#include <SFML/Graphics.hpp>
#include <SFUI/Scrollbar.hpp>
#include <thread>
#include <vector>

class Link;
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

	static AllAppsListState* Instance() 
	{
		return &AllAppsListState_dontfuckwithme;
	}

protected:
	AllAppsListState() { }

private:
	static AllAppsListState AllAppsListState_dontfuckwithme;
	AppEngine* app;

	sf::View *mainView;
	sf::View *cardScroller;
	SFUI::Scrollbar scrollbar;

	sf::RectangleShape tracker1;
	sf::RectangleShape tracker2;
	sf::RectangleShape tracker3;

	std::vector<std::thread> threads;
	std::vector<StoreApp*> apps;
	std::vector<Link*> links;

	//TODO: remove these

	void loadApps(bool &finisehdIndicator);

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

#endif // !ALL_APPS_LIST_STATE_HPP