#ifndef TOOL_TIP_HPP
#define TOOL_TIP_HPP

#include <SFML/Graphics.hpp>

class Tooltip : public sf::Drawable
{
public:
	void setPosition(sf::Vector2f pos);
	void setString(std::string string, float windowSize);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::Text text;
	sf::ConvexShape arrow;
	sf::RectangleShape box;

	const float padding = 2;

	void wrapSfText(sf::Text& target, const float width);
};

#endif // !TOOL_TIP_HPP
