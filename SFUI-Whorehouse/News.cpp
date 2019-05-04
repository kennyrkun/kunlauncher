#include "News.hpp"

#include "Globals.hpp"

#include <iostream>

// FIXME: This is a really slow function.
void wrapSfText(sf::Text& target, const float width)
{
	std::string str = target.getString();

	// no need to wrap
	if (target.getLocalBounds().width < width)
		return;

	const float containerWidth = width;
	for (auto i = 0u; i < target.getString().getSize(); ++i)
	{
		// position of the character greater than the width of the view we're wrapping for
		if (target.findCharacterPos(i).x >= containerWidth)
		{
			int s = str.rfind(' ', i);
			// try to wrap before the sequence starts, but if a space can't be found, wrap here.
			str.insert(s != std::string::npos ? s : i, "\n");
			target.setString(str);
		}
	}
}

// TODO: make a class for "aware text" that can auto wrapSfText and unwrap and stuff

News::News(std::string titlestr, std::string textstr, sf::RenderWindow* window) : titlestr(titlestr), original_TextString(textstr)
{
	title.setFont(*GBL::theme.getFont("Arial.ttf"));
	title.setCharacterSize(24);
	title.setString(titlestr);
	divider.setSize(sf::Vector2f(title.getLocalBounds().width, 2));

	text.setFont(SFUI::Theme::getFont());
	text.setCharacterSize(14);
	text.setString(original_TextString);

	wrapSfText(text, window->getSize().x - (20 + text.getPosition().x));
	display_TextString = text.getString();
}

News::~News()
{

}

void News::setPosition(const sf::Vector2f& pos)
{
	title.setPosition(pos);

	divider.setPosition(sf::Vector2f(pos.x + 3, (title.getPosition().y + title.getLocalBounds().height) + 20));

	text.setPosition(sf::Vector2f(pos.x, (divider.getPosition().y + divider.getLocalBounds().height) + 10));
}

sf::Vector2f News::getPosition()
{
	return title.getPosition();
}

float News::getLocalHeight()
{
	return (text.getPosition().y + text.getLocalBounds().height) - title.getPosition().y;
}

void News::HandleEvents(const sf::Event& event)
{
	if (event.type == sf::Event::EventType::Resized)
	{
		text.setString(original_TextString);
		float height = getLocalHeight(); // old height so we can see if we need to reposition the texts

		wrapSfText(text, event.size.width - (20 + text.getPosition().x));
		display_TextString = text.getString();

		if (height < getLocalHeight())
			std::cout << "NEWS: we need to re position the texts" << std::endl;
	}
}

void News::Update()
{
}

void News::draw(sf::RenderTarget& target, sf::RenderStates) const
{
	target.draw(title);
	target.draw(divider);
	target.draw(text);
}
