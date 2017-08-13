#include "Modal.hpp"

#include <iostream>

#include <SFUI\Button.hpp>

bool Modal:: mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))))
		return true;
	else
		return false;
}

Modal::Modal(ModalOptions settings_)
{
	settings = settings_;

	sf::Font font;
	sf::Text text;
	if (font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
	{
		text.setFont(font);
		text.setString(settings.text);
		text.setCharacterSize(18);
		text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
	}
	else
	{
		std::cerr << "unable to load font" << std::endl;
		abort();
	}

	window.create(sf::VideoMode(text.getLocalBounds().width + 18, settings.height), settings.title, sf::Style::Titlebar);
	window.setVerticalSyncEnabled(true);
	text.setPosition(static_cast<int>(window.getDefaultView().getCenter().x), static_cast<int>(text.getCharacterSize()));

	std::vector<SFUI::Button*> buttons;
	for (size_t i = 0; i < settings.settings.size(); i++)
	{
		SFUI::Button *newButton = new SFUI::Button(settings.settings[i]);

		if (i == 0)
		{
			std::cout << "first shape x is " << newButton->m_shape.getGlobalBounds().width << std::endl;

			newButton->setPosition(sf::Vector2f((window.getDefaultView().getCenter().x * 2) - (newButton->m_shape.getLocalBounds().width / 2) - 4, static_cast<int>(window.getDefaultView().getCenter().y + 20)));
		}
		else
		{
			sf::RectangleShape &last = buttons[i - 1]->m_shape;

			std::cout << "shape " << i - 1 << " x is " << last.getSize().x << std::endl;
			std::cout << "shape " << i - 1 << " y is " << last.getSize().y << std::endl;
			std::cout << "current is: " << settings.settings[i] << std::endl;

			newButton->setPosition(sf::Vector2f(last.getPosition().x - last.getLocalBounds().width / 2 - newButton->m_shape.getLocalBounds().width / 2 - 4, last.getPosition().y));
		}

		buttons.push_back(newButton);
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::EventType::Closed)
			{
				window.close();
			}
			else if (event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (event.key.code == sf::Mouse::Left)
				{
					for (size_t i = 0; i < buttons.size(); i++)
					{
						if (mouseIsOver(buttons[i]->m_shape))
						{
							returnCode = i;

							return;
						}
					}
				}
			}
		}

		for (size_t i = 0; i < buttons.size(); i++)
		{
			if (mouseIsOver(buttons[i]->m_shape))
			{
				buttons[i]->setButtonColor(sf::Color(200, 200, 200));
			}
			else
			{
				buttons[i]->setButtonColor(sf::Color(240, 240, 240));
			}
		}

		window.clear(sf::Color(30, 30, 30));

		window.draw(text);

		for (size_t i = 0; i < buttons.size(); i++)
			buttons[i]->draw(window);

		window.display();
	}
}

Modal::~Modal()
{
	// do stuff
}

// public:



// private: