#ifndef NEWS_EDIT_STATE_HPP
#define NEWS_EDIT_STATE_HPP

#include "VerticalScrollbar.hpp"

#include "../SFUI-Whorehouse/AppState.hpp"
#include "MultilineInputBox.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/InputBox.hpp>

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

// TODO: include scrollbar
// TODO: also add scrollbar to applistate

class NewsEditState : public AppState
{
public:
	NewsEditState(const std::string& newsToEdit);

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

	const std::string newsToEdit;

	int newsStartPositionInFile = -1;

	SFUI::Menu* menu;
	SFUI::InputBox* titleBox;
	MultilineInputBox* contentBox;
	
	std::string title;
	std::string content;

	void createMenu(SFUI::Menu& menu);

	void loadNewsContent(const std::string& title);
	void overwriteOldNewsContent();
	void downloadNewsFiles();

	bool mouseIsOver(sf::Shape &object);
};

#endif // !APP_LIST_STATE_HPP
