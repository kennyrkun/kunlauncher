#include "ToolTip.hpp"

#include <SFUI/Theme.hpp>

void Tooltip::setPosition(sf::Vector2f pos)
{
	float width = 20;
	arrow.setPointCount(3);
	arrow.setPoint(0, sf::Vector2f(width / 2, 0));
	arrow.setPoint(1, sf::Vector2f(0, width / 2));
	arrow.setPoint(2, sf::Vector2f(width / 2, width));
	arrow.setFillColor(sf::Color::Black);

	arrow.setPosition(sf::Vector2f(pos.x + 10, pos.y));
	box.setPosition(sf::Vector2f(pos.x + width, pos.y));
	text.setPosition(sf::Vector2f(pos.x + width + padding, pos.y + padding));
}

void Tooltip::setString(std::string string, float windowSize)
{
	text.setFont(SFUI::Theme::getFont());
	text.setCharacterSize(12);
	text.setString(string);
	wrapSfText(text, windowSize);

	box.setSize(sf::Vector2f(text.getLocalBounds().width + (padding * 3), text.getLocalBounds().height + 4 + (padding * 2)));
	box.setFillColor(sf::Color::Black);
}

void Tooltip::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(box);
	target.draw(text);
	target.draw(arrow);
}

void Tooltip::wrapSfText(sf::Text& target, const float width)
{
	std::string str = target.getString();

	//	const float containerWidth = target.getCharacterSize();
	const float containerWidth = width;
	for (size_t i = 0; i < target.getString().getSize(); ++i)
	{
		if (target.findCharacterPos(i).x >= containerWidth)
		{
			str.insert(i, "\n");
			target.setString(str);
		}
	}
}