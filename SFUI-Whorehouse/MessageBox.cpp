#include "MessageBox.hpp"
#include "Globals.hpp"

#include <SFUI/TextButton.hpp>
#include <iostream>
#include <functional>

// public:

MessageBox::MessageBox(Options settings_)
{
	settings = settings_;

	if (font.loadFromFile(GBL::DIR::fonts + "Arial.ttf"))
	{
		message.setFont(font);
		message.setString(settings.text);
		message.setCharacterSize(18);
	}
	else
	{
		if (font.loadFromFile("C://Windows//Fonts//Arial.ttf"))
		{
			message.setFont(font);
			message.setString(settings.text);
			message.setCharacterSize(18);
		}
		else
		{
			std::cerr << "unable to load font" << std::endl;
			abort();
		}

		//TODO: handle this better
	}

	//TODO: refactor this

	float messageWidth = message.getLocalBounds().width + 20;
	float buttonsWidth = 0;

	for (size_t i = 0; i < buttons.size(); i++)
		buttonsWidth += buttons[i]->m_shape.getLocalBounds().width + 4.0f;

	std::vector<float> combinedWidths;
	combinedWidths.push_back(messageWidth);

	for (size_t i = 0; i < settings.settings.size(); i++)
	{
		std::cout << "adding button for option \"" << settings.settings[i] << "\"" << std::endl;

		SFUI::TextButton *newButton = new SFUI::TextButton(settings.settings[i]);
		// we will set their positions later, because if we do it now they won't know where to go because the window hasn't been created
		// and they don't know how big it'll be and my god this is all just a huge mess and kill me now
		buttons.push_back(newButton);

		buttonsWidth += newButton->m_shape.getLocalBounds().width + 4;
	}

	combinedWidths.push_back(buttonsWidth + 6.0f);

	float largest = combinedWidths.front();
	for (size_t i = 0; i < combinedWidths.size(); i++)
		if (combinedWidths[i] > largest)
			largest = combinedWidths[i];

	if (largest < 900.0f)
	{
		settings.width = largest;
	}
	else
	{
		std::cout << "modal is too wide, setting it to 900" << std::endl;
		settings.width = 900.0f;
	}

	if (message.getLocalBounds().height < 600.0f)
	{
		settings.height = message.getLocalBounds().height + buttons.back()->m_shape.getLocalBounds().height + 30.0f;
	}
	else
	{
		std::cout << "modal is too tall, setting it to 600" << std::endl;
		settings.height = 600.0f;
	}
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
	message.setFillColor(GBL::COLOR::TEXT);

	{ // put buttons in their proper positions
		buttons[0]->setPosition(sf::Vector2f(window.getSize().x - (buttons[0]->m_shape.getLocalBounds().width / 2) - 4, window.getSize().y - 16));
//		buttons[0]->setPosition(sf::Vector2f((window.getView().getCenter().x * 2) - (buttons[0]->m_shape.getLocalBounds().width / 2) - 4, static_cast<int>(window.getView().getCenter().y + 22)));
		for (size_t i = 1; i < settings.settings.size(); i++)
		{
			sf::RectangleShape &last = buttons[i - 1]->m_shape;
			buttons[i]->setPosition(sf::Vector2f(last.getPosition().x - (last.getLocalBounds().width / 2) - (buttons[i]->m_shape.getLocalBounds().width / 2) - 4, last.getPosition().y));
		}
	}

	int selectedButtonNum = 0;
	selectedButton = buttons[0];
	buttons[0]->select();

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
			else if (event.type == sf::Event::EventType::MouseButtonPressed)
			{
				for (size_t i = 0; i < buttons.size(); i++)
				{
					if (mouseIsOver(buttons[i]->m_shape))
					{
						selectedButton->deselect();
						selectedButton = buttons[i];
						selectedButtonNum = i;
						buttons[i]->select();

						break;
					}
				}
			}
			else if (event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (event.key.code == sf::Mouse::Left)
				{
					for (size_t i = 0; i < buttons.size(); i++)
					{
						if (mouseIsOver(buttons[i]->m_shape))
						{
							selectedButton->deselect();
							selectedButton = buttons[i];
							selectedButtonNum = i;
							buttons[i]->select();

							returnCode = i;

							return;
						}
					}
				}
			}
			else if (event.type == sf::Event::EventType::MouseMoved)
			{
				for (size_t i = 0; i < buttons.size(); i++)
				{
					if (mouseIsOver(buttons[i]->m_shape))
					{
						buttons[i]->setButtonColor(sf::Color(226, 245, 255));
					}
					else
					{
						buttons[i]->setButtonColor(sf::Color(240, 240, 240));
					}
				}
			}
			else if (event.type == sf::Event::EventType::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Key::Return)
				{
					std::cout << "enter pressed" << std::endl;
				}
			}
			else if (event.type == sf::Event::EventType::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Key::Return)
				{
					std::cout << "enter released" << std::endl;

					returnCode = selectedButtonNum;
					return;
				}
				else if (event.key.code == sf::Keyboard::Key::Left)
				{
					selectedButton->deselect();

					if ((selectedButtonNum + 1) >= buttons.size())
					{
						selectedButton = buttons.front();
						selectedButtonNum = 0;
					}
					else
					{
						selectedButton = buttons[selectedButtonNum + 1];
						selectedButtonNum += 1;
					}

					selectedButton->select();
				}
				else if (event.key.code == sf::Keyboard::Key::Right)
				{
					selectedButton->deselect();

					if ((selectedButtonNum - 1) < 0)
					{
						selectedButton = buttons.back();
						selectedButtonNum = buttons.size() - 1;
					}
					else
					{
						selectedButton = buttons[selectedButtonNum - 1];
						selectedButtonNum -= 1;
					}

					selectedButton->select();
				}
			}
			else if (event.type == sf::Event::EventType::LostFocus)
			{
				std::cout << "dialog lost focus" << std::endl;

				if (selectedButton != nullptr)
					selectedButton->deselect();
			}
			else if (event.type == sf::Event::EventType::GainedFocus)
			{
				std::cout << "dialog gained focus" << std::endl;

				if (selectedButton != nullptr)
					selectedButton->select();
			}
		}

		window.clear(GBL::COLOR::BACKGROUND);

		window.draw(message);

		for (size_t i = 0; i < buttons.size(); i++)
			window.draw(*buttons[i]);

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
