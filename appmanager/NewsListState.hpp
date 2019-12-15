#ifndef NEWS_LIST_STATE_HPP
#define NEWS_LIST_STATE_HPP

#include "VerticalScrollbar.hpp"

#include "../SFUI-Whorehouse/AppState.hpp"
#include "SFUI/Layouts/Menu.hpp"

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// TODO: include scrollbar
// TODO: also add scrollbar to applistate

class NewsListState : public AppState
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

	// TODO: retain scroll position after menu changes
	VerticalScrollbar scrollbar;
	sf::Vector2f originalMenuPosition;
	sf::Vector2f originalThumbPosition;
	float scrollerTopPosition;
	float scrollerBottomPosition;
	float scrollerMinPosition;
	float scrollerMaxPosition;
	// updates the scrollthumb's size with new content height
	void updateScrollThumbSize();
	// updates the viewScroller's physical limits
	void updateScrollLimits();
	void testScrollBounds();
	// TODO: get rid of this
	void menuMove(const sf::Vector2f& offset)
	{
		menu->setPosition(sf::Vector2f(menu->getAbsolutePosition().x + offset.x, menu->getAbsolutePosition().y + offset.y));
	}

	SFUI::Menu* menu;

	void createMenu(SFUI::Menu& menu);

	void refreshNewsList();

	std::vector<std::string> newsList;

	bool mouseIsOver(sf::Shape &object);
};

#endif // !APP_LIST_STATE_HPP