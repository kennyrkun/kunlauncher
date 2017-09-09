#include "AppEngine.hpp"
#include "AppState.hpp"
#include "HomeState.hpp"
#include "AppListState.hpp"

#include "Download.hpp"
#include "Modal.hpp"
#include "Item.hpp"
#include "Link.hpp"
#include "Globals.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>
#include <mutex>

HomeState HomeState::IntialiseState_dontfuckwithme;

void HomeState::Init(AppEngine* app_)
{
	std::cout << "IntialiseState Init" << "\n";

	app = app_;

	font = new sf::Font();

	if (!font->loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::FONT + "\\Product Sans.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << "\n";

		if (!font->loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font!" << "\n";

			abort();
		}
	}

	app->window->setTitle("KunLauncher " + CONST::VERSION);

	homeText.setFont(*font);
	homeText.setCharacterSize(42);
	homeText.setString("click to view applist\n right click to return");
	homeText.setOrigin(homeText.getLocalBounds().width / 2, homeText.getLocalBounds().height / 2);
	homeText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y)));
}

void HomeState::Cleanup()
{
	delete font;

	std::cout << "Cleaned IntialiseState up." << "\n";
}

void HomeState::Pause()
{
	printf("IntialiseState Pause\n");
}

void HomeState::Resume()
{
	std::cout << "IntialiseState Resume" << "\n";
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
			std::cout << "new width: " << event.size.width << "\n";
			std::cout << "new height: " << event.size.height << "\n";

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::View newView = sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height));
				app->window->setView(newView);

				newView.setSize(event.size.width, event.size.height);
				newView.setCenter(newView.getSize().x / 2, newView.getSize().y / 2);
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
		else if (event.type == sf::Event::EventType::MouseButtonPressed)
		{
			if (event.key.code == sf::Mouse::Button::Left)
			{
				app->ChangeState(AppListState::Instance());
			}
		}
	}
}

void HomeState::Update()
{

}

void HomeState::Draw()
{
	app->window->clear(CONST::COLOR::BACKGROUND);

	app->window->draw(homeText);

	app->window->display();
}
