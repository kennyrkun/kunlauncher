#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include <SFUI/Layouts/Menu.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

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

	int exitCode;

private:
	Options settings;
	sf::RenderWindow window;

	std::string title;

	sf::Font font;
	sf::Text message;
	std::string messageString;

	SFUI::Menu* menu;
};

#endif // !MESSAGEBOX_HPP


/*
#ifndef MESSAGEBOX_HPP
#define MESSAGEBOX_HPP

#include <SFUI/Layouts/Menu.hpp>
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// TODO: preset button and return code combos
// MessageBox::Options::Buttons = 1
// 1 = Yes, No
// returns MessageBox::Buttons::Yes

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

	int runBlocking();

	void close();

	int exitCode;

private:
	Options settings;
	sf::RenderWindow window;

	std::string title;

	sf::Font font;
	sf::Text message;
	std::string messageString;

	SFUI::Menu* menu;
};

#endif // !MESSAGEBOX_HPP
*/