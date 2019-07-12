#ifndef APP_HPP
#define APP_HPP

#include "AppInfo.hpp"

#include <SFML/Graphics.hpp>

// TODO: show info instead of download button
// TODO: show visual info box like KunLauncher

class App
{
public:
	App(sf::RenderWindow* target_window, float xSize, float ySize, float xPos, float yPos);
	~App();

	AppInfo info;

	void draw();

	float totalHeight;

	sf::RectangleShape cardShape;

	sf::RenderWindow* targetWindow;

	sf::Texture		   iconTexture;
	sf::RectangleShape icon;

	sf::RectangleShape downloadButton;
	sf::Texture downloadButtonTexture;

	sf::Text	name;
	sf::Text	description;
	sf::Text	version;
};

#endif // !APP_HPP
