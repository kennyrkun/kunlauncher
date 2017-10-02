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
	std::cout << "creating scrollbar" << "\n";

	targetWindow = target_window;

	scrollbar.setSize(sf::Vector2f(10, targetWindow->getSize().y));
	scrollbar.setOrigin(sf::Vector2f(scrollbar.getLocalBounds().width / 2, 0));
	scrollbar.setPosition((targetWindow->getView().getCenter().x * 2) - scrollbar.getLocalBounds().width / 2, targetWindow->getView().getCenter().y);
	scrollbar.setFillColor(sf::Color(80, 80, 80));
		
	scrollThumb.setSize(sf::Vector2f(scrollbar.getSize().x, targetWindow->getSize().y));
	scrollThumb.setOrigin(sf::Vector2f(scrollThumb.getLocalBounds().width / 2, 0));
	scrollThumb.setPosition((targetWindow->getView().getCenter().x * 2) - scrollThumb.getLocalBounds().width / 2, targetWindow->getView().getCenter().y);
	scrollThumb.setFillColor(sf::Color(110, 110, 110));

	std::cout << "scrollbar thumb is " << scrollThumb.getSize().x << " by " << scrollThumb.getSize().y << "\n";

	scrollJumpMultiplier = 16;
}

void Scrollbar::update(float contentHeight_, float viewportHeight)
{
	std::cout << "updating scrollbar" << "\n";

	scrollbar.setSize(sf::Vector2f(10, targetWindow->getSize().y));
	scrollbar.setOrigin(sf::Vector2f(scrollbar.getLocalBounds().width / 2, 0));
	scrollbar.setPosition(sf::Vector2f((targetWindow->getView().getCenter().x * 2) - scrollbar.getLocalBounds().width / 2, 0));

	scrollThumb.setSize(sf::Vector2f(scrollbar.getSize().x, targetWindow->getSize().y));
	scrollThumb.setOrigin(sf::Vector2f(scrollThumb.getLocalBounds().width / 2, 0));
	scrollThumb.setPosition(sf::Vector2f((targetWindow->getView().getCenter().x * 2) - scrollThumb.getLocalBounds().width, targetWindow->getView().getCenter().y / 2));

	contentHeight = contentHeight_;
	min = scrollbar.getPosition().y;
	max = contentHeight;

	float viewableRatio = viewportHeight / contentHeight; // 1/3 or 0.333333333n
	float thumbHeight = viewportHeight * viewableRatio; // 50px

	// calculate scroll step
	float scrollTrackSpace = contentHeight - viewportHeight;
	float scrollThumbSpace = viewportHeight - thumbHeight;
	scrollJump = (scrollTrackSpace / scrollThumbSpace) * scrollJumpMultiplier;

	scrollThumb.setSize(sf::Vector2f(scrollThumb.getSize().x, thumbHeight));
	scrollThumb.setOrigin(sf::Vector2f(scrollThumb.getLocalBounds().width / 2, 0));
	scrollThumb.setPosition(scrollbar.getPosition());

	scrollThumbTopPosition	  = scrollThumb.getPosition().y;
	scrollThumbBottomPosition = scrollThumb.getPosition().y + scrollThumb.getLocalBounds().height;
	scrollbarTopPosition	  = scrollbar.getPosition().y;
	scrollbarBottomPosition   = scrollbar.getPosition().y + scrollbar.getLocalBounds().height;

	if (scrollThumb.getSize().y >= scrollbar.getSize().y)
	{
		isEnabled = false;
		scrollbar.setSize(sf::Vector2f(0, 0));
		std::cout << "scrollbar disabled" << std::endl;
	}
	else
	{
		isEnabled = true;
		std::cout << "scrollbar enabled" << std::endl;
	}
}

bool Scrollbar::canScrollDown()
{
	return (scrollThumbBottomPosition < scrollbarBottomPosition);
}

void Scrollbar::moveThumbDown()
{
	if (scrollThumbBottomPosition < scrollbarBottomPosition)
	{
		scrollThumb.move(0, 1 * scrollJumpMultiplier);

		updateLimits();

		if (scrollThumbBottomPosition > scrollbarBottomPosition) // clamp
		{
			std::cout << "scrollbar went too far down (" << scrollThumbBottomPosition << ":" << scrollbarBottomPosition << "), clamping..." << std::endl;
			scrollThumb.setPosition(scrollThumb.getPosition().x, scrollbarBottomPosition - scrollThumb.getLocalBounds().height);
			updateLimits();
		}
	}
	else
	{
		std::cout << "cannot scroll down (" << scrollThumbBottomPosition << " < " << scrollbarBottomPosition << ")" << std::endl;
	}
}

bool Scrollbar::canScrollUp()
{
	return (scrollThumbTopPosition > scrollbarTopPosition);
}

void Scrollbar::moveThumbUp()
{
	if (scrollThumbTopPosition > scrollbarTopPosition)
	{
		scrollThumb.move(0, -1 * scrollJumpMultiplier);

		updateLimits();

		if (scrollThumbTopPosition < scrollbarTopPosition) // clamp
		{
			std::cout << "scrollbar went too far up (" << scrollThumbTopPosition << ":" << scrollbarTopPosition << "), clamping..." << std::endl;
			scrollThumb.setPosition(scrollThumb.getPosition().x, scrollbarTopPosition);
			updateLimits();
		}
	}
	else
	{
		std::cout << "cannot scroll up (" << scrollThumbTopPosition << " < " << scrollbarTopPosition << ")" << std::endl;
	}
}

void Scrollbar::moveToTop()
{
}

void Scrollbar::moveToBottom()
{
}

void Scrollbar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(scrollbar);
	target.draw(scrollThumb);
}

void Scrollbar::updateLimits()
{
	scrollThumbTopPosition = scrollThumb.getPosition().y;
	scrollThumbBottomPosition = scrollThumb.getPosition().y + scrollThumb.getLocalBounds().height;
	scrollbarTopPosition = scrollbar.getPosition().y;
	scrollbarBottomPosition = scrollbar.getPosition().y + scrollbar.getLocalBounds().height;
}
