#include "MessageBox.hpp"

#include <iostream>

#include <SFUI\Button.hpp>

// public:

MessageBox::MessageBox(MessageBoxOptions settings_)
{
	settings = settings_;

	if (font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
	{
		message.setFont(font);
		message.setString(settings.text);
		message.setCharacterSize(18);
	}
	else
	{
		std::cerr << "unable to load font" << "\n";
		abort();

		//TODO: handle this better
	}

	int messageWidth = message.getLocalBounds().width + 20;
	int buttonsWidth = 0;

	for (size_t i = 0; i < buttons.size(); i++)
	{
		buttonsWidth += buttons[i]->m_shape.getLocalBounds().width + 4;
	}

	std::vector<int> combinedWidths;
	combinedWidths.push_back(messageWidth);

	for (size_t i = 0; i < settings.settings.size(); i++)
	{
		std::cout << "adding button for option \"" << settings.settings[i] << "\"" << "\n";

		SFUI::Button *newButton = new SFUI::Button(settings.settings[i]);
		// we will set their positions later, because if we do it now they won't know where to go because the window hasn't been created
		// and they don't know how big it'll be and my god this is all just a huge mess and kill me now
		buttons.push_back(newButton);

		buttonsWidth += newButton->m_shape.getLocalBounds().width + 4;
	}

	combinedWidths.push_back(buttonsWidth + 6);

	int largest = combinedWidths.front();
	for (size_t i = 0; i < combinedWidths.size(); i++)
		// it's greater than the previous largest
		if (combinedWidths[i] > largest)
			// it's the new largest
			largest = combinedWidths[i];
		else
			std::cout << i << " not the largest" << "\n";

	if (largest < 900)
	{
		settings.width = largest;
	}
	else
	{
		std::cout << "modal is too wide, setting it to 900" << "\n";
		settings.width = 900;
	}

	if (message.getLocalBounds().height < 600)
	{
		settings.height = message.getLocalBounds().height + buttons.back()->m_shape.getLocalBounds().height + 30;
	}
	else
	{
		std::cout << "modal is too tall, setting it to 600" << "\n";
		settings.height = 600;
	}

	std::cout << "largest number is " << largest << "\n";
}

MessageBox::~MessageBox()
{
	// do stuff
}

void MessageBox::runBlocking()
{
	window.create(sf::VideoMode(settings.width, settings.height), settings.title, sf::Style::Titlebar);
	window.setVerticalSyncEnabled(true);
	message.setPosition(static_cast<int>(window.getView().getCenter().x), static_cast<int>(message.getCharacterSize()));
	message.setPosition(7, 5);

	{ // put buttons in their proper positions
		buttons[0]->setPosition(sf::Vector2f(window.getSize().x - (buttons[0]->m_shape.getLocalBounds().width / 2) - 4, window.getSize().y - 16));
//		buttons[0]->setPosition(sf::Vector2f((window.getView().getCenter().x * 2) - (buttons[0]->m_shape.getLocalBounds().width / 2) - 4, static_cast<int>(window.getView().getCenter().y + 22)));
		for (size_t i = 1; i < settings.settings.size(); i++)
		{
			sf::RectangleShape &last = buttons[i - 1]->m_shape;
			buttons[i]->setPosition(sf::Vector2f(last.getPosition().x - (last.getLocalBounds().width / 2) - (buttons[i]->m_shape.getLocalBounds().width / 2) - 4, last.getPosition().y));
		}
	}

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::EventType::Closed)
			{
				window.close();

				returnCode = -1;
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
			else if (sf::Event::EventType::MouseMoved)
			{
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
			}
		}

		window.clear(sf::Color(30, 30, 30));

		window.draw(message);

		for (size_t i = 0; i < buttons.size(); i++)
			window.draw(*buttons[i]);
//			buttons[i]->draw(window);

		window.display();
	}
}

void MessageBox::close()
{
	window.close();
	buttons.clear();
}

// private:

bool MessageBox::mouseIsOver(const sf::Shape &object)
{
	if (object.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))))
		return true;
	else
		return false;
}
