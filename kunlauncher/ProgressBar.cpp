#include "ProgressBar.hpp"

#include <iostream>

ProgressBar::ProgressBar(sf::Vector2f position, float width, float height)
{
	box.setPosition(position);

	bar[0].position = sf::Vector2f(position.x + 0, position.y + 0);
	bar[1].position = sf::Vector2f(position.x + width, position.y + 0);
	bar[3].position = sf::Vector2f(position.x + 0, position.y + height);
	bar[2].position = sf::Vector2f(position.x + width, position.y + height);

	for (int i = 0; i < 4; ++i)
		bar[i].color = barColor;
}

void ProgressBar::addThingToDo()
{
//	std::cout << "Mr. ProgressBar: Added one more task." << std::endl;

	thingsToDo += 1;
	setValue(thingsDone, thingsToDo);
}

void ProgressBar::addThingsToDo(signed int things)
{
//	std::cout << "Mr. Progressbar: Added " << things << " more tasks" << std::endl;

	thingsToDo += things;
	setValue(thingsDone, thingsToDo);
}

void ProgressBar::oneThingDone()
{
	setValue(thingsDone + 1, thingsToDo);
}

void ProgressBar::setColor(const sf::Color& color)
{
	for (int i = 0; i < 4; ++i)
		bar[i].color = color;
}

void ProgressBar::setColor(const sf::Color & topLeft, const sf::Color & topRight, const sf::Color & bottomLeft, const sf::Color & bottomRight)
{
	bar[0].color = topLeft;
	bar[1].color = topRight;
	bar[3].color = bottomLeft;
	bar[2].color = bottomRight;
}

void ProgressBar::setValue(int thingsDone_)
{
	thingsDone = thingsDone_;
	percentDone = calculatePercent(thingsDone, thingsToDo);

	bar[1].position.x = percentDone * 4;
	bar[2].position.x = percentDone * 4;

//	std::cout << "Mr. ProgressBar: " << percentDone << "% (" << thingsDone << "/" << thingsToDo << ")" << std::endl;
}

void ProgressBar::setValue(int thingsDone_, int thingsToDo_)
{
	thingsDone = thingsDone_;
	thingsToDo = thingsToDo_;
	percentDone = calculatePercent(thingsDone, thingsToDo);

	bar[1].position.x = percentDone * 4;
	bar[2].position.x = percentDone * 4;
	
//	std::cout << "Mr. ProgressBar: " << percentDone << "% (" << thingsDone << "/" << thingsToDo << ")" << std::endl;
}

float ProgressBar::getValue() const
{
	return percentDone;
}

void ProgressBar::reset()
{
	setValue(0, 0);
}

void ProgressBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(bar, 4, sf::Quads, states);
}

float ProgressBar::calculatePercent(float x, float y)
{
	return x / y * 100;
}
