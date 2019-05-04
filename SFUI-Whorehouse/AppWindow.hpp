#ifndef APP_WINDOW_HPP
#define APP_WINDOW_HPP

#include "App.hpp"

namespace SFUI {
	class Menu;
}

class VisualItemInfo
{
public:
	VisualItemInfo();
	~VisualItemInfo();

	// creates and opens the window
	// creates all relevant information
	void updateMenu(ItemInfo info);

	void open(ItemInfo info);
	bool isOpen() { return window.isOpen(); }

	void close();

	void HandleEvents();
	// redraws the window if necessary
	void Update();

	void focus();

	enum PANEL_CALLBACK
	{
		Download,
		Delete,
		OpenLocal
	};

	std::vector<int> eventQueue;

private:
	sf::RenderWindow window;
	bool redrawRequired;

	SFUI::Menu* menu;
	void buildMenu();

	void setIcon();

	ItemInfo info;

	sf::RectangleShape image;
	sf::Texture image_Tex;
	sf::Image image_Icon;
};

#endif // !APP_WINDOW_HPP
