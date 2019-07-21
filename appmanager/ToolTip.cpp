#include "ToolTip.hpp"

#include <SFUI/Theme.hpp>

Tooltip::Tooltip()
{
	text.setFont(SFUI::Theme::getFont());
}

void Tooltip::setPosition(const sf::Vector2f& pos)
{
	float width = 20;
	arrow.setPointCount(3);
	arrow.setPoint(0, sf::Vector2f(left ? width / 2 : 0, 0));
	arrow.setPoint(1, sf::Vector2f(left ? 0 : width / 2, width / 2));
	arrow.setPoint(2, sf::Vector2f(left ? width / 2 : 0, width));
	arrow.setFillColor(sf::Color::Black);

	box.setPosition(sf::Vector2f(pos.x + (width / 2), pos.y));
	arrow.setPosition(sf::Vector2f(pos.x + (left ? 0 : box.getSize().x + (width / 2)), pos.y));
	text.setPosition(sf::Vector2f(pos.x + (width / 2) + padding, pos.y + padding));

//	wrapSfText(text, windowSize);
}

void Tooltip::setString(std::string string)
{
	text.setCharacterSize(12);
	text.setString(string);

	box.setSize(sf::Vector2f(text.getLocalBounds().width + (padding * 3), text.getLocalBounds().height + 4 + (padding * 2)));
	box.setFillColor(sf::Color::Black);
}

void Tooltip::setLeft()
{
	left = true;
}

void Tooltip::setRight()
{
	left = false;
}

sf::Vector2f Tooltip::getSize()
{
	return sf::Vector2f(box.getSize().x + arrow.getGlobalBounds().width, box.getSize().y);
}

void Tooltip::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(box);
	target.draw(text);
	target.draw(arrow);
}

/*
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
*/