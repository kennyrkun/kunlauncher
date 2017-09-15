#include <fstream>
#include <iostream>
#include <experimental\filesystem>

#include "Globals.hpp"
#include "Link.hpp"
#include "Download.hpp"

Link::Link(std::string text, std::string uri, sf::RenderWindow* target_window, float xPos)
{
	targetWindow = target_window;
	linkText.setString(text);
	linkRel = uri;

	std::cout << "creating new card for \"" << linkText.getString().toAnsiString() << "\" linking to \"" << linkRel << "\"" << "\n";

	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5, xPos)); // probably not the best
	cardShape.setFillColor(CONST::COLOR::LINK::CARD);

	font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	linkText.setFont(font);
	linkText.setPosition(static_cast<int>((cardShape.getPosition().x - cardShape.getLocalBounds().width / 2) + 15), static_cast<int>(cardShape.getPosition().y - cardShape.getLocalBounds().height / 2));
	linkText.setStyle(sf::Text::Underlined);
	linkText.setFillColor(CONST::COLOR::LINK::TEXT); // standard link blue, I think.

	// don't center them because they may get longer or shorter, but they need to be left aligned
	followLinkTexture.loadFromFile(".\\" + CONST::DIR::BASE + CONST::DIR::RESOURCE + CONST::DIR::TEXTURE + "input_1x.png");
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
#else
	std::cout << "This function is not supported on your platform!" << "\n";
#endif
}

void Link::update()
{
	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5, cardShape.getPosition().x)); // probably not the best

	totalHeight = cardShape.getLocalBounds().height;

	linkText.setPosition(static_cast<int>((cardShape.getPosition().x - cardShape.getLocalBounds().width / 2) + 15), static_cast<int>(cardShape.getPosition().y - cardShape.getLocalBounds().height / 2));

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
