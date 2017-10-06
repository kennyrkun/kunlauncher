#ifndef PROGRESS_BAR_HPP
#define PROGRESS_BAR_HPP

#include <SFML\Graphics.hpp>

class ProgressBar : public sf::Drawable
{
public:
	ProgressBar(sf::Vector2f position, float width = 200.f, float height = 20.0f);

	void addThingToDo();
	void addThingsToDo(int things);
	void oneThingIsDone();
	void setValue(int thingsDone_);
	void setValue(int thingsDone_, int thingsToDo_);
	float getValue() const;

	int thingsToDo;
	int thingsDone;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::RectangleShape  box;
	sf::Vertex		    bar[4];

	const sf::Color barColor = sf::Color(150, 150, 150);
	const float borderSize = 1.0f;
	const float textSize = 12.0f;
	const float PADDING = 1.f;

	float percentDone;

	float calculatePercent(float x, float y);
};

#endif