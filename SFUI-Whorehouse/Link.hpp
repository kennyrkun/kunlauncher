#ifndef LINK_HPP
#define LINK_HPP

#include <SFML\Graphics.hpp>

class Link 
{
public:
	Link(std::string raw, sf::RenderWindow* target_window, float xPos);
	~Link();

	sf::RectangleShape cardShape;
	sf::RectangleShape followLinkButton;
	std::string linkRel;
	sf::Text linkText;
	int cardNumber; // used to store depth

	void follow();
	void draw();

private:
	sf::RenderWindow* targetWindow;

	sf::Texture followLinkTexture;

	sf::Font font;

	std::ifstream& GotoLine(std::ifstream& file, unsigned int num);
	void getLink(std::string from_string);
};

#endif