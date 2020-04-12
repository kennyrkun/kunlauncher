#ifndef COLOR_INPUT_BOX_HPP
#define COLOR_INPUT_BOX_HPP

#include <SFUI/Widget.hpp>
#include <SFUI/Utils/Box.hpp>

// TODO: support Hex colors

class ColorInputBox : public SFUI::Widget
{
public:
	ColorInputBox(float width = 200.f);

	void setText(const sf::String& string);

	void setColor(const sf::Color &color);
	sf::Color getColor() const { return m_color.getFillColor(); };

	bool isEmpty() const;

	const sf::String& getText() const;

	void setCursorPosition(size_t index);
	size_t getCursorPosition() const;

	void setBlinkPeriod(float period);
	float getBlinkPeriod() const;

	std::string RGBtoStr(sf::Color rgb);

protected:
	// Callbacks
	void onKeyPressed(sf::Keyboard::Key key);
	void onMousePressed(float x, float y);
	void onTextEntered(sf::Uint32 unicode);

	// FIXME: sometimes these keep the hovered state
	void onStateChanged(SFUI::State state);

private:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	// TODO: don't remove spaces from string, silently ignore them.
	// FIXME: this whole function setup is nasty. 
	//		  improve readability and logical flow.
	sf::Color textToRGB();

	SFUI::Box         m_box;

	sf::RectangleShape m_color;

	std::string m_oldText;
	sf::Text    m_text;

	mutable sf::RectangleShape m_cursor;
	mutable sf::Clock  m_cursor_timer;
	float			   m_blink_period;
	// TODO: add cursor delay (.25s)

	size_t m_max_characters;
	size_t m_cursor_pos;
};

#endif // !COLOR_INPUT_BOX_HPP
