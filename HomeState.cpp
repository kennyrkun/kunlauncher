#include "HomeState.hpp"

#include "AppEngine.hpp"
#include "AppState.hpp"
#include "AllAppsListState.hpp"
#include "SettingsState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "App.hpp"
#include "Section.hpp"

#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

HomeState HomeState::HomeState_dontfuckwithme;

void wrap(const sf::Text& target, const int width)
{
	std::cout << "wrapping string from " << width << std::endl;

	const sf::Text base = target;
	const std::string baseString = target.getString();

	sf::Text modify = base;
	std::string modifyString;

	modify.setString("");
	for (size_t i = 0; i < baseString.size(); i++)
	{
		std::cout << "at character " << i << ", " << baseString[i] << std::endl;

		modifyString = modify.getString() + baseString[i];
		modify.setString(modifyString);

		if (modify.getLocalBounds().width > width)
		{
			std::cout << "wrapping 1 line" << std::endl;

			std::string s = modify.getString();
			s.insert(i, "\n");

			modify.setString(s);
		}
		else
		{
			std::cout << "no wrap" << std::endl;

			std::cout << modify.getLocalBounds().width << ":" << width << std::endl;
		}
	}

	std::cout << "string wrapped" << std::endl;
}

void HomeState::Init(AppEngine* app_)
{
	std::cout << "HomeState Init" << std::endl;

	app = app_;

	font.loadFromFile(GBL::DIR::fonts + "Arial.ttf");
	
	homeText.setFont(font);
	homeText.setString("Lorem ipsum dolor sit amet, consectetur adipiscing elit.Fusce non tempor tortor.Quisque dapibus, nibh at feugiat placerat, est lectus mattis urna, vel volutpat ipsum dui at nulla.Nulla nec odio finibus, aliquam orci ac, vehicula dui.Cras finibus tellus eget neque tincidunt, nec elementum mi pretium.Integer consectetur nunc quis erat condimentum ultricies.Proin in dui rhoncus, pharetra lectus sed, tempus lorem.Morbi non diam metus.Maecenas risus tellus, venenatis nec fringilla eget, commodo id urna.Nunc lectus nibh, auctor non est vestibulum, euismod sagittis tortor.Curabitur a felis quis justo imperdiet consectetur.\nQuisque facilisis mollis mauris, non sagittis nulla.Nam non nisl id augue porta scelerisque non sed purus.Morbi consequat mattis velit, quis euismod nisl.Orci varius natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus.Maecenas dictum, lectus pellentesque rutrum luctus, dolor enim tristique mi, sed pretium tortor ipsum at ante.Nulla ultricies id nunc ac blandit.Vivamus eget lacus finibus libero sollicitudin vestibulum id nec enim.Etiam vel semper metus, ut finibus ipsum.Maecenas eget malesuada nulla, nec sodales lorem.Donec imperdiet ut nunc sit amet imperdiet.Curabitur placerat lorem odio, id ornare nisi porttitor in.\nAliquam faucibus iaculis velit, vel pellentesque turpis mattis nec.In vel tempor elit.Maecenas tristique congue ex, sed vestibulum quam vulputate quis.Integer dictum volutpat dignissim.Nunc tristique dolor ac porta elementum.Quisque tristique ac dui ac tempus.Vivamus ac ex a elit hendrerit ullamcorper at et justo.Proin pulvinar pellentesque ligula sed dictum.Morbi placerat tempor erat, auctor facilisis quam consequat sed.Sed et venenatis arcu.Vestibulum pulvinar imperdiet arcu, non faucibus ante eleifend quis.Aliquam lacus turpis, vulputate lobortis eros quis, blandit rutrum orci.Cras quis enim eget dolor luctus aliquet.\nDuis cursus imperdiet sapien, quis mollis nisi pellentesque in.Vestibulum tincidunt fringilla eros at cursus.Sed quis cursus libero.Praesent maximus nisi leo, at tristique ligula porta sit amet.Phasellus finibus nunc vel felis placerat, id molestie justo luctus.Cras maximus enim sed auctor ornare.Pellentesque habitant morbi tristique senectus et netus et malesuada fames ac turpis egestas.\nCras in iaculis purus, in feugiat eros.Nam dignissim nisl sem, a aliquet felis scelerisque ac.Donec et sollicitudin lectus.Cras pharetra lorem fringilla, facilisis tortor id, imperdiet tortor.Vivamus suscipit laoreet quam sed iaculis.Maecenas convallis metus dui, at dictum eros luctus ac.Pellentesque in lacinia urna.Maecenas lacinia nisi elit, ut tristique augue hendrerit vel.Integer leo lectus, placerat et arcu vitae, dignissim vehicula purus.Curabitur venenatis lacus a elit tincidunt posuere.In hac habitasse platea dictumst.Donec mollis diam id neque consectetur, sit amet dictum velit porta.Vestibulum malesuada dolor ut enim dapibus dictum ac consequat lorem.Phasellus mattis, lacus ac ullamcorper tincidunt, risus lorem fringilla nisl, vel blandit orci erat a nisl.Mauris dignissim aliquam varius.");
//	wrap(homeText, app->window->getSize().x);

	std::cout << "HomeState ready" << std::endl;
}

void HomeState::Cleanup()
{
	std::cout << "Cleaning up HomeState" << std::endl;

	if (helperRunning)
	{
		std::cout << "waiting on helper thread to finish" << std::endl;
		helperThread->join();
	}

	sections.clear();

	std::cout << "HomeState Cleanup" << std::endl;
}

void HomeState::Pause()
{
	std::cout << "HomeState Pause" << std::endl;
}

void HomeState::Resume()
{
	std::cout << "HomeState Resume" << std::endl;
}

void HomeState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
		}
		else if (event.type == sf::Event::EventType::Resized)
		{
			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				app->window->setView(sf::View(visibleArea));
			}
			else
			{
				if (event.size.width <= 525)
					newSize.x = 525;

				if (event.size.height <= 325)
					newSize.y = 325;

				app->window->setSize(newSize);
			}
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::Escape)
			{
				app->PopState();
			}
		}
	}
}

void HomeState::Update()
{
}

void HomeState::Draw()
{
	app->window->clear(GBL::COLOR::BACKGROUND);

	app->window->draw(homeText);

	app->window->display();
}

bool HomeState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

bool HomeState::mouseIsOver(sf::Shape &object, sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool HomeState::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}