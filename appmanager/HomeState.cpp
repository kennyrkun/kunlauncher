#include "../SFUI-Whorehouse/AppEngine.hpp"
#include "HomeState.hpp"
#include "AppUploadState.hpp"

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
	UPLOAD,
	EDIT
};

void HomeState::Init(AppEngine* app_)
{
	std::cout << "HomeState Init" << std::endl;
	app = app_;

	app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));

	menu = new SFUI::Menu(*app->window);

	SFUI::HorizontalBoxLayout* hbox1 = menu->addHorizontalBoxLayout();
	hbox1->addButton("Upload", MenuCallbacks::UPLOAD);
	hbox1->addButton("Edit", MenuCallbacks::EDIT);

	SFUI::HorizontalBoxLayout* hbox2 = menu->addHorizontalBoxLayout();
	hbox2->addButton("Quit", MenuCallbacks::QUIT);
	hbox2->setPosition(sf::Vector2f(hbox2->getSize().x / 2 + 4, hbox2->getPosition().y));

	menu->setPosition(sf::Vector2f((app->window->getSize().x / 2) - menu->getSize().x / 2, (app->window->getSize().y / 2) - menu->getSize().y / 2));

	std::cout << "HomeState ready" << std::endl;
}

void HomeState::Cleanup()
{
	std::cout << "Cleaning up HomeState" << std::endl;

	if (app->multithreaded_process_running)
	{
		std::cout << "waiting on helper thread to finish" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = true;
		app->multithreaded_process_running = false;
		delete app->multithread;
	}

	delete menu;

	std::cout << "HomeState Cleanup" << std::endl;
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

			app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));
		}

		int id = menu->onEvent(event);
		switch (id)
		{
		case MenuCallbacks::EDIT:
//			app->ChangeState(new AppUploadState);
			break;
		case MenuCallbacks::UPLOAD:
			app->ChangeState(new AppUploadState);
			break;
		case MenuCallbacks::QUIT:
			app->Quit();
			break;
		}
	}
}

void HomeState::Update()
{
	if (app->multithreaded_process_finished)
	{
		std::cout << "helper thread finished work, joining" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = false;
		app->multithreaded_process_running = false;

		delete app->multithread;
	}
}

void HomeState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	app->window->draw(*menu);

	if (app->multithreaded_process_running)
		app->ShowMultiThreadedIndicator();

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