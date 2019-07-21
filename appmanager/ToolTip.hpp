#ifndef TOOL_TIP_HPP
#define TOOL_TIP_HPP

#include <SFML/Graphics.hpp>

class Tooltip : public sf::Drawable
{
public:
	Tooltip();

	void setPosition(const sf::Vector2f& pos);
	void setString(std::string string);

	void setLeft();
	void setRight();

	sf::Vector2f getSize();

	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::Text text;
	sf::ConvexShape arrow;
	sf::RectangleShape box;

	bool left = true;

	const float padding = 2;
};

#endif // !TOOL_TIP_HPP
