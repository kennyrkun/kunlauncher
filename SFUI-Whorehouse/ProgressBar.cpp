#include "ProgressBar.hpp"

#include <iostream>

ProgressBar::ProgressBar(sf::Vector2f position, float width, float height)
{
	box.setPosition(position);
//	box.setSize(sf::Vector2f(width, height));

	bar[0].position = sf::Vector2f(position.x + 0, position.y + 0);
	bar[1].position = sf::Vector2f(position.x + width, position.y + 0);
	bar[3].position = sf::Vector2f(position.x + 0, position.y + height);
	bar[2].position = sf::Vector2f(position.x + width, position.y + height);

	for (int i = 0; i < 4; ++i)
		bar[i].color = barColor;

	setValue(0, 100);

//	setSelectable(false);
}

void ProgressBar::oneThingDone()
{
	setValue(thingsDone + 1, thingsToDo);
}

void ProgressBar::setValue(float thingsDone_)
{
	thingsDone = thingsDone_;
	percentDone = getPercent(thingsDone, thingsToDo);

	bar[1].position.x = percentDone * 4;
	bar[2].position.x = percentDone * 4;

	std::cout << "Mr. ProgressBar: " << percentDone << "% (" << thingsDone << "/" << thingsToDo << ")" << std::endl;
}

void ProgressBar::setValue(float thingsDone_, float thingsToDo_)
{
	thingsDone = thingsDone_;
	thingsToDo = thingsToDo_;
	percentDone = getPercent(thingsDone, thingsToDo);

	bar[1].position.x = percentDone * 4;
	bar[2].position.x = percentDone * 4;
	
	std::cout << "Mr. ProgressBar: " << percentDone << "% (" << thingsDone << "/" << thingsToDo << ")" << std::endl;
}

float ProgressBar::getValue() const
{
	return percentDone;
}

void ProgressBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(bar, 4, sf::Quads, states);
}

float ProgressBar::getPercent(float x, float y)
{
	return x / y * 100;
}
