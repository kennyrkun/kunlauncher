#include "Navbar.hpp"

#include "Globals.hpp"

#include <iostream>

Navbar::Navbar(sf::RenderWindow* window) : window(window)
{
	std::cout << "creating navbar" << std::endl;

	bar.setSize(sf::Vector2f(window->getSize().x, 40));
	bar.setFillColor(GBL::COLOR::PRIMARY);

	std::cout << "navbar ready" << std::endl;
}

Navbar::~Navbar()
{
	std::cout << "destroying navbar" << std::endl;

	std::cout << "destroying destroyed" << std::endl;
}

void Navbar::addSection(std::string text)
{
	std::cout << "adding section \"" << text << "\"" << std::endl;

	NavbarSection* newSection = new NavbarSection(text, sections.size());

	if (sections.empty())
		newSection->text.setPosition(sf::Vector2f(10, 0));
	else
		newSection->text.setPosition(sections.back()->text.getPosition().x + sections.back()->text.getLocalBounds().width + 34, 0);

	//	if (newSection->text.getPosition())
	//	{

	//	}

	sections.push_back(newSection);
}

void Navbar::removeSection(int sectionNum)
{
	std::cout << "removing section " << sectionNum << std::endl;

	if (sections[sectionNum] == nullptr)
	{
		std::cout << "section " << sectionNum << " does not exist" << std::endl;
		return;
	}

	// I don't know if this actually works
	delete sections[sectionNum];
	sections.erase(std::remove(sections.begin(), sections.end(), sections[sectionNum]), sections.end());
}

void Navbar::HandleEvents(const sf::Event & event)
{
	if (event.type == sf::Event::EventType::Resized)
	{
		bar.setSize(sf::Vector2f(event.size.width, 40));
	}
}

void Navbar::Update()
{
}

void Navbar::Draw()
{
	window->draw(bar);

	for (size_t i = 0; i < sections.size(); i++)
		window->draw(sections[i]->text);
}