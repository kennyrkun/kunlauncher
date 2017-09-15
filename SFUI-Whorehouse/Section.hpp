#ifndef SECTION_HPP
#define SECTION_HPP

#include <SFML\Graphics.hpp>

class Section
{
public:
	Section(std::string text, std::string uri, sf::RenderWindow* target_window, float xPos, bool usable_);
	~Section();

	sf::RectangleShape cardShape;
	sf::RectangleShape followSectionButton;
	std::string forwardStateName;
	sf::Text title;
	bool usable;
	int totalHeight;

	void follow();
	void update();
	void draw();

private:
	sf::RenderWindow* targetWindow;

	sf::Texture followSectionTexture;

	sf::Font font;

	std::ifstream& GotoLine(std::ifstream& file, unsigned int num);
	void getSection(std::string from_string);
};

#endif // !SECTION_HPP
