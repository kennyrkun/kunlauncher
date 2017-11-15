#include "AppEngine.hpp"
#include "AppState.hpp"
#include "SettingsState.hpp"
#include "HomeState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "LauncherUpdater.hpp"
#include "ProgressBar.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>

SettingsState SettingsState::IntialiseState_dontfuckwithme;

namespace fs = std::experimental::filesystem;

void SettingsState::Init(AppEngine* app_)
{
	std::cout << "IntialiseState Init" << std::endl;
	app = app_;

	sf::Texture icon;
	icon.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "error_1x.png");
	
	button = new SFUI::IconButton;

	button->setIconTexture(icon, false);
	button->setButtonSize(sf::Vector2f(96, 96));
	button->setButtonColor(sf::Color(100, 100, 100));
}

void SettingsState::Cleanup()
{
	std::cout << "SettingsState Cleaningup" << std::endl;

	std::cout << "SettingsState Cleanedup" << std::endl;
}

void SettingsState::Pause()
{
	std::cout << "SettingsState Pause" << std::endl;
}

void SettingsState::Resume()
{
	std::cout << "SettingsState Resume" << std::endl;
}

void SettingsState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
		}
		else if (event.type == sf::Event::EventType::MouseButtonPressed)
		{
			if (event.key.code == sf::Mouse::Button::Right)
			{
				app->ChangeState(HomeState::Instance());
			}
		}
	}
}

void SettingsState::Update()
{
	r += dr;
	g += dg;
	b += db;

	if (r == 255 && g == 0 && b == 0)
	{
		dr = 0; dg = 1; db = 0;
	}

	if (r == 255 && g == 255 && b == 0)
	{
		dr = -1; dg = 0; db = 0;
	}

	if (r == 0 && g == 255 && b == 0)
	{
		dr = 0; dg = 0; db = 1;
	}

	if (r == 0 && g == 255 && b == 255)
	{
		dr = 0; dg = -1; db = 0;
	}

	if (r == 0 && g == 0 && b == 255)
	{
		dr = 1; dg = 0; db = 0;
	}

	if (r == 255 && g == 0 && b == 255)
	{
		dr = 0; dg = 0; db = -1;
	}
}

void SettingsState::Draw()
{
	app->window->clear(sf::Color(r, g, b));

	app->window->draw(*button);

	app->window->display();
}
