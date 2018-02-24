#include "News.hpp"

#include "Globals.hpp"

#include <iostream>

void wrap(sf::Text& target, const float width)
{
	std::cout << "wrapping string from " << width << std::endl;

	std::string str = target.getString();

	//	const float containerWidth = target.getCharacterSize();
	const float containerWidth = width;
	for (auto i = 0u; i < target.getString().getSize(); ++i)
	{
		if (target.findCharacterPos(i).x >= containerWidth)
		{
			//str.insert(str.rfind(' ', i), "\n");
			str.insert(i - 1, "-");
			str.insert(i, "\n");
			target.setString(str);
		}
	}

	std::cout << "string wrapped" << std::endl;
}


News::News(std::string titlestr, std::string textstr, sf::RenderWindow* window) : titlestr(titlestr), textstr(textstr), window(window)
{
	font.loadFromFile(GBL::DIR::fonts + "Arial.ttf");

	title.setFont(font);
	title.setCharacterSize(26);
	title.setString(titlestr);

	text.setFont(font);
	text.setCharacterSize(14);
	text.setString(textstr);

	divider.setSize(sf::Vector2f(window->getSize().x - 40, 2));

	wrap(text, window->getSize().x - 40);
}

News::~News()
{

}

void News::setPosition(const sf::Vector2f& pos)
{
	title.setPosition(pos);

	divider.setPosition(sf::Vector2f(title.getPosition().x + 10, title.getPosition().y + title.getLocalBounds().height + 15));

	text.setPosition(sf::Vector2f(title.getPosition().x, title.getPosition().y + title.getLocalBounds().height + 20));
}

sf::Vector2f News::getPosition()
{
	return title.getPosition();
}

float News::getLocalHeight()
{
	return (text.getPosition().y + text.getLocalBounds().height) - title.getPosition().y;
}

void News::HandleEvents(const sf::Event & event)
{
}

void News::Update()
{
}

void News::Draw()
{
	window->draw(title);
	window->draw(divider);
	window->draw(text);
}