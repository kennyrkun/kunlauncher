#ifndef NEWS_EDIT_STATE_HPP
#define NEWS_EDIT_STATE_HPP

#include "../SFUI-Whorehouse/AppState.hpp"
#include "SFUI/Layouts/Menu.hpp"

#include <SFML/Graphics.hpp>
#include <vector>

class NewsEditState : public AppState
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

	SFUI::Menu* menu;

	void createMenu(SFUI::Menu& menu);

	void populateApplist();

	void redownloadAppsList();

	std::vector<std::string> newsList;

	std::string getAppName(int appid);

	int downloadApp(int appid);
	int downloadIcon(int appid);
	int downloadInfo(int appid);
	int downloadFiles(int appid);

	bool mouseIsOver(sf::Shape &object);
	// view: runs the test relative to a view
	bool mouseIsOver(sf::Shape &object, sf::View* view);
	bool mouseIsOver(sf::Text &object);
};

#endif // !APP_LIST_STATE_HPP