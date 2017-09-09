
#include <fstream>
#include <iostream>
#include <experimental\filesystem>

#include "Globals.hpp"

#include "Link.hpp"
#include "Download.hpp"

Link::Link(std::string raw, sf::RenderWindow* target_window, float xPos)
{
	targetWindow = target_window;
	getLink(raw);

	std::cout << "creating new card for \"" << linkText.getString().toAnsiString() << "\" linking to \"" << linkRel << "\"" << "\n";

	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5, xPos)); // probably not the best
	cardShape.setFillColor(sf::Color(100, 100, 100));

	font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	linkText.setFont(font);
//	linkText.setPosition(sf::Vector2f(cardShape.getPosition().x - 240, cardShape.getPoint(0).y + 5));
	linkText.setPosition(sf::Vector2f(cardShape.getPosition().x - 242, cardShape.getPosition().y - 22));
	linkText.setStyle(sf::Text::Underlined);
	linkText.setFillColor(sf::Color(0, 170, 232));

	// don't center them because they may get longer or shorter, but they need to be left aligned
	followLinkTexture.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::TEXTURE + "\\input_1x.png");
	followLinkTexture.setSmooth(true);
	followLinkButton.setTexture(&followLinkTexture);
	followLinkButton.setSize(sf::Vector2f(24, 24));
	followLinkButton.setOrigin(sf::Vector2f(followLinkButton.getLocalBounds().width / 2, followLinkButton.getLocalBounds().height / 2));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	followLinkButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	totalHeight = cardShape.getLocalBounds().height;

	std::cout << "card is ready" << "\n";
}

Link::~Link()
{
	delete targetWindow;
}

void Link::follow()
{
	std::cout << "\n" << "going to " << linkRel << "\n";

#if defined (_WIN32) // one day it'll be cross platform... one day.
	std::string test = "start " + linkRel;
	system((test).c_str());
#endif
}

void Link::update()
{
	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5, cardShape.getPosition().x)); // probably not the best

	totalHeight = cardShape.getLocalBounds().height;

	linkText.setPosition(sf::Vector2f(cardShape.getPosition().x - 242, cardShape.getPosition().y - 22));

	followLinkButton.setSize(sf::Vector2f(24, 24));
	followLinkButton.setOrigin(sf::Vector2f(followLinkButton.getLocalBounds().width / 2, followLinkButton.getLocalBounds().height / 2));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	followLinkButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));
}

void Link::draw()
{
	targetWindow->draw(cardShape);
	targetWindow->draw(linkText);
	targetWindow->draw(followLinkButton);
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
