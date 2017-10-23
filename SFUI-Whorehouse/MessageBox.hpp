#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include <string>
#include <vector>

#include <SFML\Graphics.hpp>

namespace SFUI { class TextButton; }

struct MessageBoxOptions
{
	std::string title;
	std::string text;
	std::vector<std::string> settings;
	int width = 300;
	int height = 75;
};

class MessageBox
{
public:
	MessageBox(MessageBoxOptions settings_);
	~MessageBox();

	void runBlocking();

	void close();
	int returnCode;

private:
	MessageBoxOptions settings;
	sf::RenderWindow window;

	std::string title;

	sf::Font font;
	sf::Text message;
	std::string messageString;
	std::vector<SFUI::TextButton*> buttons;

	bool mouseIsOver(const sf::Shape &object);
};

#endif // !MESSAGEBOX_HPP
