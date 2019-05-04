#include "MessageBox.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/Layouts/HorizontalBoxLayout.hpp>
#include <SFUI/Theme.hpp>

#include <iostream>
#include <functional>

// public:

MessageBox::MessageBox(Options settings_)
{
	settings = settings_;

	font = SFUI::Theme::getFont();

	message.setFont(font);
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

		window.clear(SFUI::Theme::windowBgColor);
		window.draw(message);
		window.draw(*menu);
		window.display();
	}
}

void MessageBox::close()
{
	window.close();
}
