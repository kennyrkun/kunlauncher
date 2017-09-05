#include "Scrollbar.hpp"

#include <iostream>

Scrollbar::Scrollbar()
{
	// do stuff
}

Scrollbar::~Scrollbar()
{
	// do stuff
}

void Scrollbar::create(sf::RenderWindow* target_window)
{
	std::cout << "creating scrollbar" << std::endl;

	targetWindow = target_window;

	scrollbar.setSize(sf::Vector2f(10, targetWindow->getSize().y));
	scrollbar.setOrigin(sf::Vector2f(scrollbar.getLocalBounds().width / 2, scrollbar.getLocalBounds().height / 2));
	scrollbar.setPosition((targetWindow->getView().getCenter().x * 2) - scrollbar.getLocalBounds().width / 2, targetWindow->getView().getCenter().y);
	scrollbar.setFillColor(sf::Color(80, 80, 80));
		
	scrollThumb.setSize(sf::Vector2f(scrollbar.getSize().x, targetWindow->getSize().y));
	scrollThumb.setOrigin(sf::Vector2f(scrollThumb.getLocalBounds().width / 2, scrollThumb.getLocalBounds().height / 2));
	scrollThumb.setPosition((targetWindow->getView().getCenter().x * 2) - scrollThumb.getLocalBounds().width, targetWindow->getView().getCenter().y);
	scrollThumb.setFillColor(sf::Color(110, 110, 110));

	std::cout << "scrollbar thumb is " << scrollThumb.getSize().x << " by " << scrollThumb.getSize().y << std::endl;

	scrollJumpMultiplier = 16;
}

void Scrollbar::update(float contentHeight_, float viewportHeight)
{
	std::cout << "updating scrollbar" << std::endl;

	scrollbar.setSize(sf::Vector2f(10, targetWindow->getSize().y));
	scrollbar.setOrigin(sf::Vector2f(scrollbar.getLocalBounds().width / 2, scrollbar.getLocalBounds().height / 2));
	scrollbar.setPosition((targetWindow->getView().getCenter().x * 2) - scrollbar.getLocalBounds().width / 2, targetWindow->getView().getCenter().y);

	scrollThumb.setSize(sf::Vector2f(scrollbar.getSize().x, targetWindow->getSize().y));
	scrollThumb.setOrigin(sf::Vector2f(scrollThumb.getLocalBounds().width / 2, scrollThumb.getLocalBounds().height / 2));
	scrollThumb.setPosition((targetWindow->getView().getCenter().x * 2) - scrollThumb.getLocalBounds().width, targetWindow->getView().getCenter().y);

	contentHeight_ += 8; // padding at the end
	contentHeight = contentHeight_;
	maxHeight = contentHeight;
	minHeight = 0;

	float viewableRatio = viewportHeight / contentHeight; // 1/3 or 0.333333333n
	float thumbHeight = viewportHeight * viewableRatio; // 50px

	// calculate scroll step
	float scrollTrackSpace = contentHeight - viewportHeight;
	float scrollThumbSpace = viewportHeight - thumbHeight;
	scrollJump = (scrollTrackSpace / scrollThumbSpace) * scrollJumpMultiplier;

	scrollThumb.setSize(sf::Vector2f(scrollThumb.getSize().x, thumbHeight));
	scrollThumb.setOrigin(scrollThumb.getLocalBounds().width / 2, scrollThumb.getLocalBounds().height / 2);
	scrollThumb.setPosition(scrollbar.getPosition().x, scrollThumb.getLocalBounds().height / 2);
}

void Scrollbar::moveThumbUp()
{
	scrollThumb.move(0, 1 * scrollJumpMultiplier);
}

void Scrollbar::moveThumbDown()
{
	scrollThumb.move(0, -1 * scrollJumpMultiplier);
}

void Scrollbar::moveToTop()
{
}

void Scrollbar::moveToBottom()
{
}

void Scrollbar::draw()
{
	targetWindow->draw(scrollbar);
	targetWindow->draw(scrollThumb);
}
