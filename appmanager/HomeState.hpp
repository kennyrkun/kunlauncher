#ifndef HOME_STATE_HPP
#define HOME_STATE_HPP

#include "../SFUI-Whorehouse/AppState.hpp"
#include "SFUI/Layouts/Menu.hpp"

#include <SFML/Graphics.hpp>
#include <vector>

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
	AppEngine * app;

	SFUI::Menu* menu;

	bool mouseIsOver(sf::Shape &object);
	// view: runs the test relative to a view
	bool mouseIsOver(sf::Shape &object, sf::View* view);
	bool mouseIsOver(sf::Text &object);
};

#endif // !HOME_STATE_HPP