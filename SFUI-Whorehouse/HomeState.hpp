#ifndef HOME_STATE_HPP
#define HOME_STATE_HPP

#include "AppState.hpp"

#include "News.hpp"
#include "VerticalScrollbar.hpp"

#include <SFML/Graphics.hpp>
#include <vector>

// FIXME: resizing too small causes infinite (or so it seems) hang

class HomeState : public AppState
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

	std::vector<std::thread> threads;
	std::vector<News*> newses;
	std::pair<int, int> newsLoadedDist {0 , 2};

	// TODO: little shape around this like android's screentips
	sf::Text homeStatus;

	sf::Text nextText;
	sf::Text previousText;
	sf::Text enableNews;

	// loadFrom: where it should start loading news (default 0)
	// loadTo: where it should stop loading news (default 10)
	void loadNews(bool &finishedIndicator, int loadFrom, int loadTo);

	sf::View* viewScroller;
	sf::View* mainView;
	VerticalScrollbar scrollbar;

	// TODO: viewable area class
	float scrollerTopPosition;
	float scrollerBottomPosition;
	float scrollerMinPosition;
	float scrollerMaxPosition;
	// updates the scrollthumb's size with new content height
	void updateScrollThumbSize();
	// updates the viewScroller's physical limits
	void updateScrollLimits();
	void testScrollBounds();

	bool mouseIsOver(sf::Shape &object);
	// view: runs the test relative to a view
	bool mouseIsOver(sf::Shape &object, sf::View* view);
	bool mouseIsOver(const sf::Text &object);
	bool mouseIsOver(sf::Text &object, sf::View* view);

	// Sets the string of the homeStatus object.
	void setStatusText(const std::string& string);
	// Sets the string and position of the homeStatus object.
	void setStatusText(const std::string& string, const sf::Vector2f& pos);
};

#endif // !HOME_STATE_HPP