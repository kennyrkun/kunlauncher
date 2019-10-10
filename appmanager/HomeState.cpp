#include "AppEngine.hpp"

#include "HomeState.hpp"
#include "AppEditState.hpp"
#include "AppListState.hpp"
#include "NewsEditState.hpp"

#include "Globals.hpp"
#include "Download.hpp"

#include <SFUI/Layouts/HorizontalBoxLayout.hpp>
#include <SFUI/Theme.hpp>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

enum MenuCallbacks
{
	QUIT,
	UPLOAD_APP,
	EDIT_APP,
	EDIT_NEWS
};

void HomeState::Init(AppEngine* app_)
{
	std::cout << "HomeState Init" << std::endl;
	app = app_;

	std::cout << "creating menu" << std::endl;

	menu = new SFUI::Menu(*app->window);

	SFUI::HorizontalBoxLayout* container = menu->addHorizontalBoxLayout();

	SFUI::VerticalBoxLayout* appButtons = container->addVerticalBoxLayout();
	appButtons->addButton("Create New App", MenuCallbacks::UPLOAD_APP);
	appButtons->addButton("Edit Existing App", MenuCallbacks::EDIT_APP);

	SFUI::VerticalBoxLayout* newsButtons = container->addVerticalBoxLayout();
	newsButtons->addButton("Edit News...", MenuCallbacks::EDIT_NEWS);

	menu->addButton("Quit", MenuCallbacks::QUIT);

	menu->setPosition(sf::Vector2f(10, 10));

	std::cout << "HomeState ready." << std::endl;
}

void HomeState::Cleanup()
{
	std::cout << "Cleaning up HomeState" << std::endl;

	delete menu;

	std::cout << "Cleaned up HomeState." << std::endl;
}

void HomeState::Pause()
{
	std::cout << "HomeState paused" << std::endl;
}

void HomeState::Resume()
{
	std::cout << "HomeState resumed" << std::endl;
}

void HomeState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			std::cout << "quitting" << std::endl;

			app->Quit();
			return;
		}
		else if (event.type == sf::Event::EventType::Resized)
		{
			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::FloatRect visibleArea(0.0f, 0.0f, event.size.width, event.size.height);
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

		int id = menu->onEvent(event);
		switch (id)
		{
		case MenuCallbacks::EDIT_NEWS:
			app->PushState(new NewsEditState);
			return;
		case MenuCallbacks::EDIT_APP:
			app->PushState(new AppListState);
			return;
		case MenuCallbacks::UPLOAD_APP:
			app->appToEdit = -1;
			app->PushState(new AppEditState);
			return;
		case MenuCallbacks::QUIT:
			app->Quit();
			return;
		}
	}
}

void HomeState::Update()
{
}

void HomeState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	app->window->draw(*menu);

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
