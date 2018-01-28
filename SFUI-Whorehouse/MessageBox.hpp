#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace SFUI { class TextButton; }

class MessageBox
{
public:
	struct Options
	{
		std::string title;
		std::string text;
		std::vector<std::string> settings;
		float width = 300.0f;
		float height = 75.0f;
	};

public:
	MessageBox(Options settings_);
	~MessageBox();

	void runBlocking();

	void close();
	int returnCode;

	SFUI::TextButton* selectedButton = nullptr;

private:
	Options settings;
	sf::RenderWindow window;

	std::string title;

	sf::Font font;
	sf::Text message;
	std::string messageString;
	std::vector<SFUI::TextButton*> buttons;

	bool mouseIsOver(const sf::Shape &object);
};

#endif // !MESSAGEBOX_HPP
