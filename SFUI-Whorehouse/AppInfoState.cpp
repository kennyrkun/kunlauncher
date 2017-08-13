#include "AppEngine.hpp"
#include "AppState.hpp"
#include "AppInfoState.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>

#include "constants.hpp"

/*
AppInfoState::AppInfoState(AppEngine2* app_)
{
	app = app_;

//	font = new sf::Font;

	if (!font2.loadFromFile(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + FONT_DIRECTORY + "\\" + "Product Sans.ttf"))
	{
		std::cout << "failed to load default font, falling back." << std::endl;

		if (!font2.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font." << std::endl;
		}
	}

	text.setFont(font2);
//	text.setFont(*font);
	text.setCharacterSize(28);
	text.setString("information state");
	text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
	text.setPosition(app->window->getDefaultView().getCenter());

	std::cout << "AppInfoState Initialised" << std::endl;
}

AppInfoState::~AppInfoState()
{
//	delete app;
	delete font;

	std::cout << "AppInfoState Cleanup" << std::endl;
}

void AppInfoState::Pause()
{
	std::cout << "AppInfoState Paused" << std::endl;
}

void AppInfoState::Resume()
{
	std::cout << "AppInfoState Resumed" << std::endl;
}

void AppInfoState::HandleEvents()
{
	sf::Event event;
	while (app->window->pollEvent(event) && app->Running())
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
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

void AppInfoState::Update()
{

}

void AppInfoState::Draw()
{
	if (app->Running())
	{
		app->window->clear(sf::Color(50, 50, 50));

		app->window->draw(text);

		app->window->display();
	}
}
*/

// AppInfoState2

//void AppInfoState2::Init(AppEngine* app_)
AppInfoState2::AppInfoState2(AppEngine2* app_, int number_)
{
	app = app_;

	number = number_;

	//	font = new sf::Font;

	if (!font2.loadFromFile(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + FONT_DIRECTORY + "\\" + "Product Sans.ttf"))
	{
		std::cout << "failed to load default font, falling back." << std::endl;

		if (!font2.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font." << std::endl;
		}
	}

	text.setFont(font2);
	//	text.setFont(*font);
	text.setCharacterSize(28);
	text.setString("information state " + std::to_string(number));
	text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
	text.setPosition(app->window->getDefaultView().getCenter());

	std::cout << "AppInfoState Initialised" << std::endl;
}

AppInfoState2::~AppInfoState2()
{
	delete app;
	delete font;

	std::cout << "AppInfoState Cleanup" << std::endl;
}

void AppInfoState2::Pause()
{
	std::cout << "AppInfoState Paused" << std::endl;
}

void AppInfoState2::Resume()
{
	std::cout << "AppInfoState Resumed" << std::endl;
}

void AppInfoState2::HandleEvents()
{
	sf::Event event;
	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			std::cout << "done" << std::endl;

			app->Quit();
			return;
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::Escape)
			{
				app->PopState();
				return;
			}
			else if (event.key.code == sf::Keyboard::Key::N)
			{
				std::cout << "switching to InfoState2" << std::endl;

				AppInfoState2* infoState = new AppInfoState2(app, (app->states.size()));
//				app->ChangeState(infoState);
				app->PushState(infoState);
				return;
			}
		}
	}
}

void AppInfoState2::Update()
{

}

void AppInfoState2::Draw()
{
	app->window->clear(sf::Color(50, 50, 50));

	app->window->draw(text);

	app->window->display();
}