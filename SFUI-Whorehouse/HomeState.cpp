#include "AppEngine.hpp"
#include "AppState.hpp"
#include "HomeState.hpp"
#include "AppListState.hpp"

#include "Download.hpp"
#include "Modal.hpp"
#include "Item.hpp"
#include "Link.hpp"
#include "constants.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>
#include <mutex>

HomeState HomeState::IntialiseState_dontfuckwithme;

void HomeState::Init(AppEngine* app_)
{
	std::cout << "IntialiseState Init" << std::endl;

	app = app_;

	if (!font.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::FONT + "\\Product Sans.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << std::endl;

		if (!font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font!" << std::endl;

			abort();
		}
	}

	app->window->setTitle("KunLauncher " + CONST::VERSION);

	homeText.setFont(font);
	homeText.setCharacterSize(42);
	homeText.setString("click for applist");
	homeText.setOrigin(homeText.getLocalBounds().width / 2, homeText.getLocalBounds().height - 20);
	homeText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y)));
}

void HomeState::Cleanup()
{
	std::cout << "Cleaned IntialiseState up." << std::endl;
}

void HomeState::Pause()
{
	printf("IntialiseState Pause\n");
}

void HomeState::Resume()
{
	std::cout << "IntialiseState Resume" << std::endl;
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
		else if (event.type == sf::Event::EventType::KeyPressed || event.type == sf::Event::EventType::MouseButtonPressed)
		{
			app->ChangeState(AppListState::Instance());
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
