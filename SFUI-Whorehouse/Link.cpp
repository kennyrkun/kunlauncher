
#include <fstream>
#include <iostream>
#include <experimental\filesystem>

#include "constants.hpp"

#include "Link.hpp"
#include "Download.hpp"

Link::Link(std::string raw, sf::RenderWindow* target_window, float xPos)
{
	targetWindow = target_window;
	getLink(raw);

	std::cout << "creating new card for \"" << linkText.getString().toAnsiString() << "\" linking to \"" << linkRel << "\"" << std::endl;

	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5, xPos)); // probably not the best
	cardShape.setFillColor(sf::Color(100, 100, 100));
//	cardShape.setOutlineColor(sf::Color::Magenta);
//	cardShape.setOutlineThickness(1.0f);

	font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	linkText.setFont(font);
	linkText.setPosition(sf::Vector2f(cardShape.getPosition().x - 230, cardShape.getPosition().y));

	std::string text = linkText.getString();
	if (text.find('p') != std::string::npos ||
		text.find('g') != std::string::npos ||
		text.find('j') != std::string::npos ||
		text.find('q') != std::string::npos ||
		text.find('y') != std::string::npos)
		linkText.setPosition(cardShape.getPosition().x - 230, cardShape.getPosition().y + 4);
	else
		linkText.setPosition(cardShape.getPosition().x - 230, cardShape.getPosition().y - 2);

	linkText.setOrigin(0, linkText.getLocalBounds().height - 1);
	linkText.setStyle(sf::Text::Underlined);
	linkText.setFillColor(sf::Color(0, 170, 232));

	// don't center them because they may get longer or shorter, but they need to be left aligned
	followLinkTexture.loadFromFile(".\\" + BASE_DIRECTORY + "\\res\\tex\\input_1x.png");
	followLinkTexture.setSmooth(true);
	followLink.setTexture(&followLinkTexture);
	followLink.setSize(sf::Vector2f(24, 24));
	followLink.setOrigin(sf::Vector2f(followLink.getLocalBounds().width / 2, followLink.getLocalBounds().height / 2));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	followLink.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	std::cout << "card is ready" << std::endl;
}

Link::~Link()
{
	delete targetWindow;
}

void Link::follow()
{
	std::cout << "\n" << "going to " << linkRel << std::endl;

#if defined (_WIN32) // one day it'll be cross platform... one day.
	std::string test = "start " + linkRel;
	system((test).c_str());
#endif
}

void Link::draw()
{
	targetWindow->draw(cardShape);
	targetWindow->draw(linkText);
	targetWindow->draw(followLink);
}

// private

std::ifstream& Link::GotoLine(std::ifstream& file, unsigned int num)
{
	file.seekg(std::ios::beg); // go to top

	for (size_t i = 0; i < num - 1; ++i) // go to line
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return file; // return line
}

void Link::getLink(std::string from_string)
{
	std::string temp = from_string;
	temp.erase(0, temp.find_first_of('"') + 1);
	temp.erase(temp.find_first_of('"'), temp.length());
	linkText.setString(temp);

	temp = from_string;
	temp.erase(0, temp.find_first_of("=") + 2);
	temp.erase(temp.find_last_of('"'), temp.length());
	linkRel = temp;
}
