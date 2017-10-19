#include "AppEngine.hpp"
#include "AppState.hpp"
#include "HomeState.hpp"
#include "AppListState.hpp"
#include "SettingsState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "Item.hpp"
#include "Section.hpp"

#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>

HomeState HomeState::HomeState_dontfuckwithme;

void HomeState::Init(AppEngine* app_)
{
	std::cout << "HomeState Init" << "\n";

	app = app_;

	if (!app->window->isOpen())
	{
		app->window->create(sf::VideoMode(app->settings.width, app->settings.height), "KunLauncher " + GBL::VERSION, sf::Style::Resize | sf::Style::Close);
		app->window->setVerticalSyncEnabled(app->settings.verticalSync);
	}

//	SFUI::IconButton* appListButton = new SFUI::IconButton;

	SFUI::IconButton *appListButton = new SFUI::IconButton;
	appListButton->setIconTexture(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "apps_2x.png", true);
	appListButton->setButtonColor(sf::Color(100, 100, 100));
	appListButton->setPosition(sf::Vector2f(app->window->getView().getCenter().x - appListButton->shape.getLocalBounds().width * 1.5 - 48, app->window->getView().getCenter().y - appListButton->shape.getLocalBounds().height / 2));
	sections.push_back(appListButton);

	SFUI::IconButton *settingsState = new SFUI::IconButton;
	settingsState->setIconTexture(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "settings_2x.png", true);
	settingsState->setButtonColor(sf::Color(100, 100, 100));
	settingsState->setPosition(sf::Vector2f(app->window->getView().getCenter().x - settingsState->shape.getLocalBounds().width / 2, app->window->getView().getCenter().y - settingsState->shape.getLocalBounds().height / 2));
	sections.push_back(settingsState);

	SFUI::IconButton *powerButton = new SFUI::IconButton;
	powerButton->setIconTexture(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "power_2x.png", true);
	powerButton->setButtonColor(sf::Color(100, 100, 100));
	powerButton->setPosition(sf::Vector2f(app->window->getView().getCenter().x + powerButton->shape.getLocalBounds().width / 2 + 48, app->window->getView().getCenter().y - powerButton->shape.getLocalBounds().height / 2));
	sections.push_back(powerButton);

	std::cout << "loaded " << sections.size() << " sections" << std::endl;
}

void HomeState::Cleanup()
{
	if (helperRunning)
	{
		std::cout << "waiting on helper thread to finish" << "\n";
		helperThread->join();
	}

	sections.clear();
	//	delete app; // dont delete app because it's being used by the thing and we need it.
	//	app = nullptr;

	std::cout << "HomeState Cleanup" << "\n";
}

void HomeState::Pause()
{
	std::cout << "HomeState Pause" << "\n";
}

void HomeState::Resume()
{
	std::cout << "HomeState Resume" << "\n";
}

void HomeState::HandleEvents(sf::Event& event)
{
	if (event.type == sf::Event::EventType::Closed)
	{
		app->Quit();
	}
	else if (event.type == sf::Event::EventType::MouseButtonPressed)
	{
		if (event.key.code == sf::Mouse::Button::Left)
		{
			//sections
			for (size_t i = 0; i < sections.size(); i++)
			{
				if (mouseIsOver(sections[0]->shape))
					app->ChangeState(AppListState::Instance());
				else if (mouseIsOver(sections[1]->shape))
					app->ChangeState(SettingsState::Instance());
				else if (mouseIsOver(sections[2]->shape))
					app->Quit();
			}
		}
	}
}

void HomeState::Update()
{
	if (helperDone && !helperRunning)
	{
		std::cout << "helper done, joining" << "\n";
		helperThread->join();

		helperDone = false;
		helperRunning = false;
	}

	for (size_t i = 0; i < threads.size(); i++)
	{
		if (threads[i].joinable())
		{
			std::cout << "joining" << "\n";

			threads[i].detach();
			threads.erase(threads.begin() + i);
		}
	}
}

void HomeState::Draw()
{
	app->window->clear(GBL::COLOR::BACKGROUND);

	for (size_t i = 0; i < sections.size(); i++)
	{
		app->window->draw(*sections[i]);
	}

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