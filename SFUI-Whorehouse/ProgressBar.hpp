#ifndef PROGRESS_BAR_HPP
#define PROGRESS_BAR_HPP

#include <SFML\Graphics.hpp>

class ProgressBar : public sf::Drawable
{
public:
	ProgressBar(sf::Vector2f position, float width = 200.f, float height = 20.0f);

	void oneThingDone();
	void setValue(float thingsDone_);
	void setValue(float thingsDone_, float thingsToDo_);
	float getValue() const;

	float thingsToDo;
	float thingsDone;
	float percentDone;

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	sf::RectangleShape  box;
	sf::Vertex		    bar[4];

	const sf::Color barColor = sf::Color(150, 150, 150);
	const float borderSize = 1.0f;
	const float textSize = 12.0f;
	const float PADDING = 1.f;

	float getPercent(float x, float y);
};

#endif