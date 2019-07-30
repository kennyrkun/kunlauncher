#include "AppEngine.hpp"

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

Navbar::Navbar(sf::RenderWindow* window, PhysicalAnimator& animator) : window(window), animator(animator)
{
	std::cout << "creating navbar" << std::endl;

	bar.setSize(sf::Vector2f(window->getSize().x, 40.0f - 1.0f));
	bar.setFillColor(GBL::theme.palatte.PRIMARY);

	divider.setSize(sf::Vector2f(bar.getSize().x, 1));
	divider.setPosition(sf::Vector2f(0, bar.getSize().y));
	divider.setFillColor(sf::Color::Black);

	selectedThing.setFillColor(sf::Color(GBL::theme.palatte.TERTIARY.r, GBL::theme.palatte.TERTIARY.g, GBL::theme.palatte.TERTIARY.b, 100));
	selectedThing.setSize(sf::Vector2f(40, bar.getSize().y));

	std::cout << "navbar ready" << std::endl;
}

Navbar::~Navbar()
{
	std::cout << "destroying navbar" << std::endl;
	
	sections.clear();

	std::cout << "navbar destroyed" << std::endl;
}

bool Navbar::select(const std::string& section)
{
	if (sections.find(section) != sections.end())
	{
		sf::Text text = sections.find(section)->second;

		//selectedThing.setSize(sf::Vector2f(text.getGlobalBounds().width + 18, bar.getSize().y));

		animator.addRectangleSizeTask(selectedThing, sf::Vector2f(text.getGlobalBounds().width + 18, bar.getSize().y), EaseType::CubicEaseOut, 500);
		animator.addTranslationTask(selectedThing, sf::Vector2f(text.getPosition().x - 8, text.getPosition().y), EaseType::CubicEaseOut, 500);

		selectedSection = section;

		std::cout << "selected " << section << std::endl;

		return true;
	}

	return false;
}

sf::Text& Navbar::addSection(const std::string& section)
{
	if (sections.find(section) != sections.end())
	{
		std::cerr << "section already exists (" << sections.find(section)->first << ")" << std::endl;

		// just because we can't return null
		abort();
	}

	std::cout << "adding section \"" << section << "\"" << std::endl;

	sf::Text newSection(sf::String(section), *GBL::theme.getFont("Arial.ttf"), 28);
	newSection.setFillColor(GBL::theme.palatte.TEXT);

	if (sections.empty())
		newSection.setPosition(sf::Vector2f(8, 0));
	else
	{
		std::map<std::string, sf::Text>::reverse_iterator it = sections.rbegin();

		sf::Text back = it->second;
		std::cout << "section before " << section << " is " << it->first << std::endl;
		newSection.setPosition(sf::Vector2f(nextXPositon, 0));
	}

	nextXPositon = newSection.getPosition().x + newSection.getGlobalBounds().width + 42;

	// TODO: adapt size and positioning of items so that the bar can accomodate any amount of entries.

	sections.emplace(section, newSection);
	return sections.at(section);
}

void Navbar::removeSection(const std::string& section)
{
	sections.erase(section);
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

		/*
		for (auto& x : sections)
		{
			if (mouseIsOverText(x, window))
			{
			}
		}
		*/
	}
}

void Navbar::Update()
{
}

void Navbar::Draw()
{
	window->draw(bar);
	window->draw(divider);

	window->draw(selectedThing);

	for (auto& x : sections)
		window->draw(x.second);

//	window->draw(border);
}
