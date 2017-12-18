#include "Globals.hpp"
#include "Section.hpp"
#include "Download.hpp"

#include <fstream>
#include <iostream>
#include <experimental/filesystem>

Section::Section(std::string text, std::string forwardSection, float xSize, float ySize, float xPos, float yPos)
{
	title.setString(text);
	forwardStateName = forwardSection;

	std::cout << "creating new section card for \"" << title.getString().toAnsiString() << "\" for \"" << forwardStateName << "\"" << std::endl;

	cardShape.setSize(sf::Vector2f(xSize, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f(xPos, yPos)); // probably not the best
	cardShape.setFillColor(sf::Color(100, 100, 100));

	font.loadFromFile("C://Windows//Fonts//Arial.ttf");
	title.setFont(font);
	title.setPosition(static_cast<int>((cardShape.getPosition().x - cardShape.getLocalBounds().width / 2) + 15), static_cast<int>(cardShape.getPosition().y - cardShape.getLocalBounds().height / 2));

	followSectionTexture.loadFromFile(".//" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "input_1x.png");
	followSectionTexture.setSmooth(true);
	followSectionButton.setTexture(&followSectionTexture);
	followSectionButton.setSize(sf::Vector2f(24, 24));
	followSectionButton.setOrigin(sf::Vector2f(followSectionButton.getLocalBounds().width / 2, followSectionButton.getLocalBounds().height / 2));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	followSectionButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	totalHeight = cardShape.getLocalBounds().height;

	std::cout << "card is ready" << std::endl;
}

Section::~Section()
{
//	nothing
}

void Section::follow()
{
	std::cout << "\n" << "going to " << forwardStateName << std::endl;

#if defined (_WIN32) // one day it'll be cross platform... one day.
	std::string test = "start " + forwardStateName;
	system((test).c_str());
#else
	std::cerr << "This function is not supported on your platform!" << std::endl;
#endif
}

void Section::update(float xSize, float ySize, float xPos, float yPos)
{
	cardShape.setSize(sf::Vector2f(xSize, 40));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f(xPos, cardShape.getPosition().y)); // probably not the best
	totalHeight = cardShape.getLocalBounds().height;

	title.setPosition(static_cast<int>((cardShape.getPosition().x - cardShape.getLocalBounds().width / 2) + 15), static_cast<int>(cardShape.getPosition().y - cardShape.getLocalBounds().height / 2));

	followSectionButton.setOrigin(sf::Vector2f(followSectionButton.getLocalBounds().width / 2, followSectionButton.getLocalBounds().height / 2));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	followSectionButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));
}

void Section::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(cardShape);
	target.draw(title);
	target.draw(followSectionButton);
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
