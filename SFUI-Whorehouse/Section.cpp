#include "Globals.hpp"
#include "Section.hpp"
#include "Download.hpp"

#include <fstream>
#include <iostream>
#include <experimental\filesystem>

Section::Section(std::string text, std::string uri, sf::RenderWindow* target_window, float xPos, bool usable_ = false)
{
	targetWindow = target_window;
	title.setString(text);
	forwardStateName = uri;

	std::cout << "creating new card for \"" << title.getString().toAnsiString() << "\" Sectioning to \"" << forwardStateName << "\"" << "\n";

	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5, xPos)); // probably not the best
	cardShape.setFillColor(sf::Color(100, 100, 100));

	font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");
	title.setFont(font);
	title.setPosition(static_cast<int>((cardShape.getPosition().x - cardShape.getLocalBounds().width / 2) + 15), static_cast<int>(cardShape.getPosition().y - cardShape.getLocalBounds().height / 2));

	followSectionTexture.loadFromFile(".\\" + CONST::DIR::BASE + CONST::DIR::RESOURCE + CONST::DIR::TEXTURE + "input_1x.png");
	followSectionTexture.setSmooth(true);
	followSectionButton.setTexture(&followSectionTexture);
	followSectionButton.setSize(sf::Vector2f(24, 24));
	followSectionButton.setOrigin(sf::Vector2f(followSectionButton.getLocalBounds().width / 2, followSectionButton.getLocalBounds().height / 2));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	followSectionButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	totalHeight = cardShape.getLocalBounds().height;

	std::cout << "card is ready" << "\n";
}

Section::~Section()
{
	delete targetWindow;
}

void Section::follow()
{
	std::cout << "\n" << "going to " << forwardStateName << "\n";

#if defined (_WIN32) // one day it'll be cross platform... one day.
	std::string test = "start " + forwardStateName;
	system((test).c_str());
#else
	std::cout << "This function is not supported on your platform!" << "\n";
#endif
}

void Section::update()
{
	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5, cardShape.getPosition().x)); // probably not the best
	totalHeight = cardShape.getLocalBounds().height;

	title.setPosition(static_cast<int>((cardShape.getPosition().x - cardShape.getLocalBounds().width / 2) + 15), static_cast<int>(cardShape.getPosition().y - cardShape.getLocalBounds().height / 2));

	followSectionButton.setSize(sf::Vector2f(24, 24));
	followSectionButton.setOrigin(sf::Vector2f(followSectionButton.getLocalBounds().width / 2, followSectionButton.getLocalBounds().height / 2));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	followSectionButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));
}

void Section::draw()
{
	targetWindow->draw(cardShape);
	targetWindow->draw(title);
	targetWindow->draw(followSectionButton);
}

// private

std::ifstream& Section::GotoLine(std::ifstream& file, unsigned int num)
{
	file.seekg(std::ios::beg); // go to top

	for (size_t i = 0; i < num - 1; ++i) // go to line
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return file; // return line
}
