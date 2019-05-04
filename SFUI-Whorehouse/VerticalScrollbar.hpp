#ifndef SCROLLBAR_HPP
#define SCROLLBAR_HPP

#include <SFML\Graphics.hpp>

// TODO: improve the way scrollbar works when things are loading or moving
// TODO: center the scrollbar
// TODO: use an sf::FloatRect for scroll area

class VerticalScrollbar : public sf::Drawable
{
public:
	VerticalScrollbar();
	~VerticalScrollbar();

	void create(sf::RenderWindow* target_window);
	void update(float contentHeight_, float viewportHeight); // recalculate scroll parameters and thumb

	void setTrackHeight(float trackHeight); // height of the track
	float trackHeight;

	// TODO: acknowledge WIDTH setting in DisplayProperties
	void setBarWidth(float barWidth);
	float barWidth = 10; // width of the track

	// the origion of the scrollbar is the top right
	void setPosition(const sf::Vector2f& pos);
	// the origion of the scrollbar is the top right
	sf::Vector2f getPosition();

	// binds a view to the scrollbar.
	void bindToView(sf::View* view);
	// removes a bound view from the scrollbar
	void unbindFromView(sf::View* view);

	bool isEnabled = false; // disables showing the scrollbar

	// View: will move the associated view with the bar
	// PixelPerfect: will round to the nearest whole number to stop bad looking text and such

	bool canThumbMoveDown(); // can the scrollthumb scroll down without going out of bounds?
	void jumpDown(sf::View* view = nullptr, bool pixelPerfect = false); // move the scrollthumb the jump distance
	void pageDown(sf::View* view = nullptr, bool pixelPerfect = false); // move the thumb down an amount equal to viewport height
	void stepDown(sf::View* view = nullptr, bool pixelPerfect = false); // move the thumb down one pixel
	void moveToBottom(sf::View* view = nullptr); // move to the bottom of the track

	bool canThumbMoveUp(); // can the scrollthumb scroll down without going out of bounds?
	void jumpUp(sf::View* view = nullptr, bool pixelPerfect = false); // move the scrollthumb the jump distance
	void pageUp(sf::View* view = nullptr, bool pixelPerfect = false); // move the thumb up an amount equal to viewport height
	void stepUp(sf::View* view = nullptr, bool pixelPerfect = false); // move the thumb up one pixel
	void moveToTop(sf::View* view = nullptr); // move to the top of the track

	// whether or not the scrollbar is within bounds
	// true if scrollbar is in bounds, otherwise false
	bool boundsCheck();

	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

	float contentSize; // Y size height of the content, in pixels
	float viewportSize; // Y size of viewport in pixels
	float viewableRatio; // amount of content that can be seen

	sf::Vector2f dragOffset; // position of the mouse relative to the origin of the scrollthumb
	sf::Vector2f originalPosition; // position of scrollTrack before being dragged
	bool draggingThumb; // is the thumb being dragged by the mouse?
	float min; // minimum thumb position, in pixels
	float max; // maximum thumb position, in pixels
	float scrollStep = 0; // amount to move the content when the thumb moves 1 pixel.
	float &scrollPage = viewportSize; // amount to move the content when the thumb moves 1 pixel.
	float scrollJump; // amount to move on one scroll. // TODO: stop using scrollStep for how much to move the scroller.
	float scrollJumpMultiplier = 16;

	float scrollbarTopPosition; // top of the physical scrollTrack
	float scrollbarBottomPosition; // bottom of the physical scrollTrack
	float scrollThumbTopPosition; // top of the physical scrollthumb
	float scrollThumbBottomPosition; // bottom of the physical scrollthumb

	sf::RectangleShape scrollTrack; // the background
	sf::RectangleShape scrollThumb; // the thing you move
	sf::RectangleShape topArrow; // top scroll step button
	sf::RectangleShape bottomArrow; // top scroll step button

private:
	sf::RenderWindow* targetWindow;

	sf::Vector2f position; // position of the physical scrollTrack's origin

	void updatePhysicalLimits(); // Updates the scrollTrack & scrollthumb's physical limits.
};

#endif // !SCROLLBAR_HPP
