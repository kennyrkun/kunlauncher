#include <SFML/OpenGL.hpp>

#include "ColorInputBox.hpp"
#include <SFUI/Theme.hpp>

#include <sstream>

ColorInputBox::ColorInputBox(float width) : m_box(SFUI::Box::Type::Input), m_cursor_pos(0), m_max_characters(-1), m_blink_period(1.0f)
{
	m_box.setSize(width, SFUI::Theme::getBoxHeight());

	int offset = SFUI::Theme::borderSize + SFUI::Theme::PADDING;
	m_text.setFont(SFUI::Theme::getFont());
	m_text.setPosition(offset, offset);
	m_text.setFillColor(SFUI::Theme::input.textColor);
	m_text.setCharacterSize(SFUI::Theme::textCharacterSize);

	m_cursor.setPosition(offset, offset);
	m_cursor.setSize(sf::Vector2f(1.f, SFUI::Theme::getLineSpacing()));
	m_cursor.setFillColor(SFUI::Theme::input.textColor);
	setCursorPosition(0);

	setText("");

	m_color.setSize(sf::Vector2f(m_box.getSize().y, m_box.getSize().y));
	m_color.setPosition(sf::Vector2f(m_box.getSize().x + 5, 0));

	setSize(sf::Vector2f(m_box.getSize().x + 5 + m_color.getSize().x, m_color.getSize().y));
}

void ColorInputBox::setText(const sf::String& string)
{
	m_text.setString(string);
	setCursorPosition(string.getSize());

	m_oldText = m_text.getString();

	m_color.setFillColor(textToRGB());
}

void ColorInputBox::setColor(const sf::Color& color)
{
	setText(RGBtoStr(color));
}

bool ColorInputBox::isEmpty() const
{
	return m_text.getString().isEmpty();
}

const sf::String& ColorInputBox::getText() const
{
	return m_text.getString();
}

void ColorInputBox::setCursorPosition(size_t index)
{
	if (index <= m_text.getString().getSize())
	{
		m_cursor_pos = index;

		float padding = SFUI::Theme::borderSize + SFUI::Theme::PADDING;
		m_cursor.setPosition(m_text.findCharacterPos(index).x, padding);
		m_cursor_timer.restart();

		if (m_cursor.getPosition().x > getSize().x - padding)
		{
			// Shift text on left
			float diff = m_cursor.getPosition().x - getSize().x + padding;
			m_text.move(-diff, 0);
			m_cursor.move(-diff, 0);
		}
		else if (m_cursor.getPosition().x < padding)
		{
			// Shift text on right
			float diff = padding - m_cursor.getPosition().x;
			m_text.move(diff, 0);
			m_cursor.move(diff, 0);
		}

		float text_width = m_text.getLocalBounds().width;

		if (m_text.getPosition().x < padding
			&& m_text.getPosition().x + text_width < getSize().x - padding)
		{
			float diff = (getSize().x - padding) - (m_text.getPosition().x + text_width);
			m_text.move(diff, 0);
			m_cursor.move(diff, 0);

			// If text is smaller than the textbox, force align on left
			if (text_width < (getSize().x - padding * 2))
			{
				diff = padding - m_text.getPosition().x;
				m_text.move(diff, 0);
				m_cursor.move(diff, 0);
			}
		}
	}
}

size_t ColorInputBox::getCursorPosition() const
{
	return m_cursor_pos;
}

void ColorInputBox::setBlinkPeriod(float period)
{
	m_blink_period = period;
}

float ColorInputBox::getBlinkPeriod() const
{
	return m_blink_period;
}

std::string ColorInputBox::RGBtoStr(sf::Color rgb)
{
//	std::cout << std::to_string(rgb.r) + "," + std::to_string(rgb.g) + "," + std::to_string(rgb.b) << std::endl;
	return std::to_string(rgb.r) + "," + std::to_string(rgb.g) + "," + std::to_string(rgb.b);
}

void ColorInputBox::onKeyPressed(sf::Keyboard::Key key)
{
	switch (key)
	{
	case sf::Keyboard::Left:
		setCursorPosition(m_cursor_pos - 1);
		break;

	case sf::Keyboard::Right:
		setCursorPosition(m_cursor_pos + 1);
		break;

	case sf::Keyboard::BackSpace:
		// Erase character before cursor
		if (m_cursor_pos > 0)
		{
			sf::String string = m_text.getString();
			string.erase(m_cursor_pos - 1);
			m_text.setString(string);

			setCursorPosition(m_cursor_pos - 1);
		}
		break;

	case sf::Keyboard::Delete:
		// Erase character after cursor
		if (m_cursor_pos < m_text.getString().getSize())
		{
			sf::String string = m_text.getString();
			string.erase(m_cursor_pos);
			m_text.setString(string);

			setCursorPosition(m_cursor_pos);
		}
		break;

	case sf::Keyboard::Home:
		setCursorPosition(0);
		break;

	case sf::Keyboard::End:
		setCursorPosition(m_text.getString().getSize());
		break;

	case sf::Keyboard::Return:
		triggerCallback();
		break;

	case sf::Keyboard::Escape:
		m_text.setString(m_oldText);
		setCursorPosition(m_text.getString().getSize());

	default:
		break;
	}
}

void ColorInputBox::onMousePressed(float x, float y)
{
	for (int i = m_text.getString().getSize(); i >= 0; --i)
	{
		// Place cursor after the character under the mouse
		sf::Vector2f glyph_pos = m_text.findCharacterPos(i);
		if (glyph_pos.x <= x)
		{
			setCursorPosition(i);
			break;
		}
	}
}

void ColorInputBox::onTextEntered(sf::Uint32 unicode)
{
	if ((unicode == 44 || unicode > 47) && unicode < 58)
	{
		sf::String string = m_text.getString();
		if (m_max_characters == -1 || static_cast<int>(string.getSize() < m_max_characters))
		{
			// Insert character in string at cursor position
			string.insert(m_cursor_pos, unicode);
			m_text.setString(string);

			setCursorPosition(m_cursor_pos + 1);

			m_color.setFillColor(textToRGB());
		}
	}
}

void ColorInputBox::onStateChanged(SFUI::State state)
{
	if (state == SFUI::State::Default)
		m_oldText = m_text.getString();

	m_box.applyState(state);
}

void ColorInputBox::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	target.draw(m_box, states);

	// Crop the text with GL Scissor
	glEnable(GL_SCISSOR_TEST);

	sf::Vector2f pos = getAbsolutePosition();
	glScissor((pos.x + SFUI::Theme::borderSize) - 1, target.getSize().y - (pos.y + getSize().y), getSize().x - 2, getSize().y);
	target.draw(m_text, states);

	glDisable(GL_SCISSOR_TEST);

	// Show cursor if focused
	if (isFocused())
	{
		// Make it blink
		float timer = m_cursor_timer.getElapsedTime().asSeconds();
		if (timer >= m_blink_period)
			m_cursor_timer.restart();

		// Updating in the drawing method, deal with it
		sf::Color color = SFUI::Theme::input.textColor;
		color.a = 255 - (255 * timer / m_blink_period);
		m_cursor.setFillColor(color);

		target.draw(m_cursor, states);
	}

	target.draw(m_color, states);
}

sf::Color ColorInputBox::textToRGB()
{
	std::string istr = getText();
	sf::Color oldColor = m_color.getFillColor();
	sf::Color newColor;

	{
		int i = istr.find(",");
		std::string r = istr.substr(0, i != std::string::npos ? i : istr.length());

		if (r.length() > 3)
			r = "255";

		std::stringstream ss(r);
		int rr;
		ss >> rr;

		if (rr > 255 || rr < 0)
			rr = 255;

		newColor.r = rr;
	}
	istr.erase(0, istr.find(","));

	{
		int begin = istr.find(",") + 1;
		int end = istr.rfind(",") - 1;

		std::string g = istr.substr(begin, end);

		if (g.length() > 3)
			g = "255";

		std::stringstream ss(g);
		int gg;
		ss >> gg;

		if (gg > 255 || gg < 0)
			gg = 255;

		newColor.g = gg;
	}
	istr.erase(0, istr.find(","));

	{
		int begin = istr.rfind(",") + 1;
		std::string b = istr.substr(begin, istr.length());

		if (b.length() > 3)
			b = "255";

		std::stringstream ss(b);
		int bb;
		ss >> bb;

		if (bb > 255 || bb < 0)
			bb = 255;

		newColor.b = bb;
	}

	return newColor;
}
