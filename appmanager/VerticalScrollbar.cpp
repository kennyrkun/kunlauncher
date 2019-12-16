#include "VerticalScrollbar.hpp"

#include "Globals.hpp"

#include <iostream>

VerticalScrollbar::VerticalScrollbar()
{
}

VerticalScrollbar::~VerticalScrollbar()
{
}

void VerticalScrollbar::create(sf::RenderWindow* target_window)
{
	std::cout << "creating scrollbar" << std::endl;

	targetWindow = target_window;

	trackHeight = target_window->getSize().y;
	position = sf::Vector2f((target_window->getView().getCenter().x * 2) - scrollTrack.getSize().x / 2, 0);
	position = sf::Vector2f((target_window->getView().getCenter().x * 2), 0);

//	scrollTrack.setSize(sf::Vector2f(10, targetWindow->getSize().y));
//	scrollTrack.setOrigin(sf::Vector2f(scrollTrack.getLocalBounds().width / 2, 0));
	scrollTrack.setPosition(position);
	scrollTrack.setFillColor(GBL::color::SCROLLBAR);

//	scrollThumb.setSize(sf::Vector2f(0, 0));
	scrollThumb.setFillColor(GBL::color::SCROLLTHUMB);
}

// TODO: don't reposition thumb after update
void VerticalScrollbar::update(float contentHeight_, float viewportHeight_)
{
	viewportSize = scrollTrack.getSize().y;
	contentSize = contentHeight_;

	scrollTrack.setSize(sf::Vector2f(barWidth, trackHeight));
	scrollTrack.setOrigin(sf::Vector2f(scrollTrack.getSize().x / 2, 0));
	scrollTrack.setPosition(sf::Vector2f(position.x - (scrollTrack.getSize().x / 2), position.y));

	min = scrollTrack.getPosition().y;
	max = contentSize;

	viewableRatio = scrollTrack.getSize().y / contentSize;
	float thumbSize = viewableRatio * scrollTrack.getSize().y;

	scrollThumb.setSize(sf::Vector2f(barWidth, thumbSize));
	scrollThumb.setOrigin(sf::Vector2f(scrollThumb.getSize().x / 2, 0));
	scrollThumb.setPosition(scrollTrack.getPosition());

	scrollThumbTopPosition = scrollThumb.getPosition().y;
	scrollThumbBottomPosition = scrollThumb.getPosition().y + scrollThumb.getSize().y;
	scrollbarTopPosition = scrollTrack.getPosition().y;
	scrollbarBottomPosition = scrollTrack.getPosition().y + scrollTrack.getSize().y;

	// TODO: don't execute anything if it's not changing
	if (viewportSize > contentSize)
	{
		isEnabled = false;

		scrollTrack.setSize(sf::Vector2f(0, trackHeight));
		scrollThumb.setSize(sf::Vector2f(0, scrollThumb.getSize().y));
	}
	else
	{
		isEnabled = true;
	}

	float scrollTrackSpace = contentSize - scrollTrack.getSize().y;
	float scrollThumbSpace = scrollTrack.getSize().y - thumbSize;
	scrollStep = (scrollTrackSpace / scrollThumbSpace);
	scrollJump = scrollStep * scrollJumpMultiplier;
}

void VerticalScrollbar::setTrackHeight(float trackHeight)
{
	this->trackHeight = trackHeight;
	scrollTrack.setSize(sf::Vector2f(scrollTrack.getSize().x, trackHeight));
	scrollTrack.setOrigin(sf::Vector2f(scrollTrack.getSize().x / 2, scrollTrack.getSize().y / 2));
}

void VerticalScrollbar::setBarWidth(float barWidth)
{
	this->barWidth = barWidth;
	scrollTrack.setSize(sf::Vector2f(barWidth, scrollTrack.getSize().y));
	scrollTrack.setOrigin(sf::Vector2f(scrollTrack.getSize().x / 2, scrollTrack.getSize().y / 2));
}

void VerticalScrollbar::setPosition(const sf::Vector2f& pos)
{
	position = pos;
	scrollTrack.setPosition(sf::Vector2f(pos.x - (scrollTrack.getLocalBounds().width / 2), pos.y));
	scrollThumb.setPosition(sf::Vector2f(scrollTrack.getPosition().x, scrollTrack.getPosition().y - scrollThumb.getPosition().y)); // why does this subtract this?
}

sf::Vector2f VerticalScrollbar::getPosition()
{
	return scrollTrack.getPosition();
}

void VerticalScrollbar::bindToView(sf::View* view)
{
}

void VerticalScrollbar::unbindFromView(sf::View* view)
{
}

bool VerticalScrollbar::canThumbMoveDown()
{
	return (scrollThumbBottomPosition < scrollbarBottomPosition);
}

void VerticalScrollbar::jumpDown(sf::View* view, bool pixelPerfect)
{
	if (canThumbMoveDown())
	{
		scrollThumb.move(0, scrollJumpMultiplier);

		updatePhysicalLimits();

		if (scrollThumbBottomPosition > scrollbarBottomPosition) // clamp
		{
			std::cerr << "scrollthumb went too far down (" << scrollThumbBottomPosition - scrollbarBottomPosition << "), clamping..." << std::endl;
			moveToBottom();
			updatePhysicalLimits();
		}
	}
	else
	{
		std::cerr << "cannot scroll down (" << scrollThumbBottomPosition << " > " << scrollbarBottomPosition << ")" << std::endl;
	}

	/*
	if (scrollerBottomPosition < scrollerMaxPosition)
		if (pixelPerfect)
			view->move(0, static_cast<int>(scrollbar.scrollJump)); // static cast to avoid pixel-imperfect placement of text
		else
			view->move(0, scrollbar.scrollJump);
	else
		std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << std::endl;

	updateScrollLimits();

	if (scrollerBottomPosition > scrollerMaxPosition) // clamp viewScroller
	{
		std::cout << "viewScroller went too far down (" << scrollerBottomPosition - scrollerMaxPosition << "), clamping..." << std::endl;
		view->setCenter(view->getCenter().x, scrollerMaxPosition - view->getSize().y / 2);
		updateScrollLimits();
	}
	*/
}

void VerticalScrollbar::pageDown(sf::View* view, bool pixelPerfect)
{
}

void VerticalScrollbar::stepDown(sf::View* view, bool pixelPerfect)
{
	if (canThumbMoveDown())
	{
		scrollThumb.move(0, 1);

		updatePhysicalLimits();

		boundsCheck();
	}
	else
	{
		std::cerr << "cannot step down (" << scrollThumbBottomPosition << " > " << scrollbarBottomPosition << ")" << std::endl;
	}
}

void VerticalScrollbar::moveToBottom(sf::View* view)
{
	scrollThumb.setPosition(scrollThumb.getPosition().x, scrollbarBottomPosition - scrollThumb.getSize().y);
}

bool VerticalScrollbar::canThumbMoveUp()
{
	return (scrollThumbTopPosition > scrollbarTopPosition);
}

void VerticalScrollbar::jumpUp(sf::View* view, bool pixelPerfect)
{
	if (canThumbMoveUp())
	{
		scrollThumb.move(0, -scrollJumpMultiplier);

		updatePhysicalLimits();

		if (scrollThumbTopPosition < scrollbarTopPosition) // clamp
		{
			std::cerr << "scrollthumb went too far up (" << scrollbarTopPosition - scrollThumbTopPosition << "), clamping..." << std::endl;
			moveToTop();
			updatePhysicalLimits();
		}
	}
	else
	{
		std::cerr << "cannot scroll up (" << scrollThumbTopPosition << " < " << scrollbarTopPosition << ")" << std::endl;
	}
}

void VerticalScrollbar::pageUp(sf::View* view, bool pixelPerfect)
{
	// TODO: pageup
	// move up viewable ratio?
}

void VerticalScrollbar::stepUp(sf::View* view, bool pixelPerfect)
{
	if (canThumbMoveUp())
	{
		scrollThumb.move(0, -1);

		boundsCheck();
	}
	else
	{
		std::cerr << "cannot step up (" << scrollThumbTopPosition << " < " << scrollbarTopPosition << ")" << std::endl;
	}
}

void VerticalScrollbar::moveToTop(sf::View* view)
{
	scrollThumb.setPosition(scrollThumb.getPosition().x, min);
}

bool VerticalScrollbar::boundsCheck()
{
	bool inBounds(true);

	updatePhysicalLimits();

	if (scrollThumbTopPosition < scrollbarTopPosition) // clamp
	{
		std::cerr << "scrollthumb went too far up (" << scrollbarTopPosition - scrollThumbTopPosition << "), clamping..." << std::endl;
		moveToTop();
		updatePhysicalLimits();

		inBounds = false;
	}

	if (scrollThumbBottomPosition > scrollbarBottomPosition) // clamp
	{
		std::cerr << "scrollthumb went too far down (" << scrollThumbBottomPosition - scrollbarBottomPosition << "), clamping..." << std::endl;
		moveToBottom();
		updatePhysicalLimits();

		inBounds = false;
	}

	return inBounds;
}

void VerticalScrollbar::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(scrollTrack, states);
	target.draw(scrollThumb, states);
}

// PRIVATE

void VerticalScrollbar::updatePhysicalLimits()
{
	scrollThumbTopPosition = scrollThumb.getPosition().y;
	scrollThumbBottomPosition = scrollThumb.getPosition().y + scrollThumb.getSize().y;
	scrollbarTopPosition = scrollTrack.getPosition().y;
	scrollbarBottomPosition = scrollTrack.getPosition().y + scrollTrack.getSize().y;
}
