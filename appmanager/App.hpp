#ifndef APP_HPP
#define APP_HPP

#include <SFML/Graphics.hpp>

// TODO: show info instead of download button
// TODO: show visual info box like KunLauncher

struct ItemInfo
{
	std::string name;
	std::string description;
	std::string version;
	std::string author;
	std::string github;
	int release;
	int appid;
};

class App
{
public:
	App(sf::RenderWindow* target_window, float xSize, float ySize, float xPos, float yPos);
	~App();

	ItemInfo info;

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
