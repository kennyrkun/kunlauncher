#ifndef SCROLLBAR_HPP
#define SCROLLBAR_HPP

#include <SFML\Graphics.hpp>

class Scrollbar
{
public:
	Scrollbar();
	~Scrollbar();

	void create(sf::RenderWindow* target_window);
	void update(float contentHeight_, float viewportHeight);

	void moveThumbUp();
	void moveThumbDown();
	void moveToTop();
	void moveToBottom();

	void draw();

	sf::RectangleShape scrollbar;
	sf::RectangleShape scrollThumb;
	bool thumbDragging;
	float scrollJump = 0;
	float scrollJumpMultiplier = 16;
	float maxHeight;
	float minHeight;
	float contentHeight;

private:
	sf::RenderWindow* targetWindow;
};

#endif