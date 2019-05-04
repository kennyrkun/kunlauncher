#include "MessageBox.hpp"

#include "Globals.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/Layouts/HorizontalBoxLayout.hpp>
#include <SFUI/Theme.hpp>

#include <iostream>
#include <functional>

// public:

MessageBox::MessageBox(Options settings_)
{
	settings = settings_;

	message.setFont(*GBL::theme.getFont("Arial.ttf"));
	message.setString(settings.text);
	message.setCharacterSize(12);
	message.setPosition(7, 5);
	message.setFillColor(sf::Color::White);

	//TODO: refactor this
	menu = new SFUI::Menu(window);

	SFUI::HorizontalBoxLayout* hbox = menu->addHorizontalBoxLayout();

	for (size_t i = 0; i < settings_.settings.size(); i++)
		hbox->addButton(settings_.settings[i], i);
}

MessageBox::~MessageBox()
{
	delete menu;

	close();
}

void MessageBox::runBlocking()
{
	if (message.getLocalBounds().width > menu->getSize().x)
		settings.width = message.getLocalBounds().width + 20;
	else
		settings.width = menu->getSize().x + 20;

	settings.height = message.getLocalBounds().height + menu->getSize().y + 30;

	window.create(sf::VideoMode(settings.width, settings.height), settings.title, sf::Style::Titlebar);
	window.setVerticalSyncEnabled(true);

	menu->setPosition(sf::Vector2f(window.getSize().x - menu->getSize().x - 10, window.getSize().y - menu->getSize().y - 10));

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			int id = menu->onEvent(event);

			if (event.type == sf::Event::EventType::Closed)
			{
				window.close();

				exitCode = -1;
				break;
			}
			// FIXME: doing this causes the message box to close if it's clicked anywhere
			else if (event.type == sf::Event::EventType::KeyPressed || event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (event.key.code == sf::Keyboard::Key::Return || event.key.code == sf::Mouse::Button::Left)
				{
					if (id < 0)
					{
						std::cout << "no widget ID was returned, skipping. (" << id << ")" << std::endl;
						break;
					}

					exitCode = id;

					close();
					return;
				}
			}
		}

		window.clear(GBL::theme.palatte.TERTIARY);
		window.draw(message);
		window.draw(*menu);
		window.display();
	}
}

void MessageBox::close()
{
	window.close();
}


/*
#include "MessageBox.hpp"
#include "Globals.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/Layouts/HorizontalBoxLayout.hpp>
#include <SFUI/Theme.hpp>

#include <iostream>
#include <functional>

// public:

//TODO: refactor MessageBox constructor
MessageBox::MessageBox(Options settings_)
{
	settings = settings_;

	if (font.loadFromFile(GBL::DIR::fonts + "Arial.ttf"))
	{
		message.setFont(font);
		message.setString(settings.text);
		message.setCharacterSize(SFUI::Theme::textCharacterSize + 4);
	}
	else
	{
		if (!font.loadFromFile("C:/Windows/Fonts/Arial.ttf"))
		{
			std::cerr << "unable to load font (mbox)" << std::endl;
			abort(); // at this point, we know it's fucked
					 // and there's nothing we can do about it.
					 // just give up.
			
		}
	}

	message.setFont(font);
	message.setString(settings.text);
	message.setCharacterSize(SFUI::Theme::textCharacterSize + 4);

	menu = new SFUI::Menu(window);

	SFUI::HorizontalBoxLayout* hbox = menu->addHorizontalBoxLayout();

	for (size_t i = 0; i < settings_.settings.size(); i++)
		hbox->addButton(settings_.settings[i], i);
}

MessageBox::~MessageBox()
{
	delete menu;

	close();
}

int MessageBox::runBlocking()
{
	window.create(sf::VideoMode(message.getLocalBounds().width > settings.width ? message.getLocalBounds().width + 20 : settings.width, 
								message.getLocalBounds().height > settings.height ? message.getLocalBounds().height + 20 : settings.height), 
								settings.title, sf::Style::Titlebar);

	window.setVerticalSyncEnabled(true);
//	message.setPosition(static_cast<int>(window.getView().getCenter().x), static_cast<int>(message.getCharacterSize()));
	message.setPosition(7, 5);
	message.setFillColor(GBL::theme.palatte.TEXT_DARK);

	menu->setPosition(sf::Vector2f(window.getSize().x - menu->getSize().x - 10, window.getSize().y - menu->getSize().y - 10));

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			int id = menu->onEvent(event);

			if (event.type == sf::Event::EventType::Closed)
			{
				window.close();

				exitCode = -1;
				break;
			}
			// FIXME: doing this causes the message box to close if it's clicked anywhere
			else if (event.type == sf::Event::EventType::KeyPressed || event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (event.key.code == sf::Keyboard::Key::Return || event.key.code == sf::Mouse::Button::Left)
				{
					if (id < 0)
					{
						std::cout << "no widget ID was returned, skipping. (" << id << ")" << std::endl;
						break;
					}

					exitCode = id;

					close();
				}
			}
		}

		window.clear(GBL::theme.palatte.TERTIARY);
		window.draw(message);
		window.draw(*menu);
		window.display();
	}

	return exitCode;
}

void MessageBox::close()
{
	window.close();
}
*/
