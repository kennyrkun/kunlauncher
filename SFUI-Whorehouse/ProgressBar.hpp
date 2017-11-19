#ifndef PROGRESS_BAR_HPP
#define PROGRESS_BAR_HPP

#include <SFML\Graphics.hpp>

class ProgressBar : public sf::Drawable
{
public:
	ProgressBar(sf::Vector2f position, float width = 200.f, float height = 20.0f);

	void addThingToDo();
	void addThingsToDo(signed int things);
	void oneThingDone();

	void setColor(const sf::Color& color);
	void setColor(const sf::Color& topLeft, const sf::Color& topRight, const sf::Color& bottomLeft, const sf::Color& bottomRight);

	void setValue(signed int thingsDone_);
	void setValue(signed int thingsDone_, signed int thingsToDo_);
	float getValue() const;
	void reset();

	signed int thingsToDo;
	signed int thingsDone;

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