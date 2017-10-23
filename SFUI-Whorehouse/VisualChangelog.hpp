#ifndef VISUAL_CHANGELOG_HPP
#define VISUAL_CHANGELOG_HPP

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

class VisualChangeLog
{
public:
	VisualChangeLog(MessageBoxOptions settings_);
	~VisualChangeLog();

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

#endif // !VISUAL_CHANGELOG_HPP
