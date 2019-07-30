#include "Navbar.hpp"

#include "Globals.hpp"

#include <iostream>

// TODO: proper sizing
// TODO: proper event checking

bool mouseIsOver(sf::Shape &object, sf::RenderWindow *window)
{
	if (object.getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window))))
		return true;
	else
		return false;
}

bool mouseIsOverText(sf::Text &object, sf::RenderWindow *window)
{
	if (object.getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window))))
		return true;
	else
		return false;
}

Navbar::Navbar(sf::RenderWindow* window) : window(window)
{
	std::cout << "creating navbar" << std::endl;

	bar.setSize(sf::Vector2f(window->getSize().x, 40.0f));
	bar.setFillColor(GBL::theme.palatte.PRIMARY);

//	border.setSize(sf::Vector2f(window->getSize().x, 1));
//	border.move(sf::Vector2f(0, bar.getSize().y));

//	shadow[0].color = sf::Color(0, 0, 0, 100);
//	shadow[1].color = sf::Color(0, 0, 0, 100);
//	shadow[2].color = sf::Color(0, 0, 0, 0);
//	shadow[3].color = sf::Color(0, 0, 0, 0);

	sf::Vector2f position = { 0, 40 };
	int width = window->getSize().x;
	int height = 20;

	shadow[0].color = sf::Color(0, 0, 0, 100);
	shadow[1].color = sf::Color(0, 0, 0, 100);
	shadow[2].color = sf::Color(0, 0, 0, 0);
	shadow[3].color = sf::Color(0, 0, 0, 0);

	shadow[0].position = sf::Vector2f(position.x, position.y);
	shadow[1].position = sf::Vector2f(position.x + width, position.y);
	shadow[3].position = sf::Vector2f(position.x, position.y + height);
	shadow[2].position = sf::Vector2f(position.x + width, position.y + height);

	std::cout << "navbar ready" << std::endl;
}

Navbar::~Navbar()
{
	std::cout << "destroying navbar" << std::endl;
	
	sections.clear();

	std::cout << "navbar destroyed" << std::endl;
}

sf::Text& Navbar::addSection(std::string text)
{
	std::cout << "adding section \"" << text << "\"" << std::endl;

	sf::Text newSection(sf::String(text), *GBL::theme.getFont("Arial.ttf"), 28);

	if (sections.empty())
		newSection.setPosition(sf::Vector2f(10, 0));
	else
		newSection.setPosition(sections.back().getPosition().x + sections.back().getLocalBounds().width + 42, 0);

	newSection.setFillColor(GBL::theme.palatte.TEXT);

	/*(
	// TODO: adapt size and positioning of items so that the bar can accomodate any amount of entries.
	if (newSection->text.getPosition())
	{
			
	}
	*/

	sections.push_back(newSection);
	return sections.back();
}

sf::Text& Navbar::getSection(std::string sectionName)
{
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].getString() == sectionName)
			return sections[i];
}

void Navbar::removeSection(int sectionNum)
{
	sections.erase(sections.begin() + sectionNum);
}

void Navbar::removeSection(std::string sectionName)
{
	for (size_t i = 0; i < sections.size(); i++)
		if (sections[i].getString() == sectionName)
		{
			sections.erase(sections.begin() + i);
			break;
		}
}

void Navbar::HandleEvents(const sf::Event& event)
{
	if (event.type == sf::Event::EventType::Resized)
	{
		bar.setSize(sf::Vector2f(event.size.width, 40.0f));
	}
	else if (event.type == sf::Event::EventType::MouseButtonPressed)
	{
		// TODO: if mouse is over the bar, see if it's over any of the items on the bar

		for (auto& x : sections)
		{
			if (mouseIsOverText(x, window))
			{
			}
		}
	}
}

void Navbar::Update()
{
}

void Navbar::Draw()
{
	window->draw(bar);
	window->draw(shadow, 4, sf::Quads);

	for (auto& x : sections)
		window->draw(x);

//	window->draw(border);
}
