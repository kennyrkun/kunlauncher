#include "AppEngine.hpp"
#include "../SFUI-Whorehouse/AppState.hpp"

#include "Globals.hpp"
#include "Download.hpp"

#include <SFUI/Theme.hpp>
#include <iostream>

void AppEngine::Init(std::string title_, AppSettings settings_)
{
	std::cout << "AppEngine Init" << std::endl;

	settings = settings_;

	window = new sf::RenderWindow;
	window->setVerticalSyncEnabled(settings.window.verticalSync);

//	LoadMultiThreadedIcon("../SFUI-Whorehouse/" + GBL::DIR::textures + "settings_2x.png"); // eventually this needs to be done in IntialiseState, but right now it refuses to work there.
	multithreaded_process_indicator.setRadius(20);
	multithreaded_process_indicator.setOrigin(sf::Vector2f(20, 20));
	multithreaded_process_indicator.setTexture(&multithreaded_process_indicator_tex);

	SFUI::Theme::loadFont(GBL::DIR::fonts + "Arial.ttf");
	SFUI::Theme::loadTexture(GBL::DIR::textures + "interface_square.png");
	SFUI::Theme::textCharacterSize = 11;
	SFUI::Theme::click.textColor = SFUI::Theme::hexToRgb("#fff");
	SFUI::Theme::click.textColorHover = SFUI::Theme::hexToRgb("#fff");
	SFUI::Theme::click.textColorFocus = SFUI::Theme::hexToRgb("#fff");
	SFUI::Theme::input.textColor = SFUI::Theme::hexToRgb("#fff");
	SFUI::Theme::input.textColorHover = SFUI::Theme::hexToRgb("#fff");
	SFUI::Theme::input.textColorFocus = SFUI::Theme::hexToRgb("#fff");
	SFUI::Theme::PADDING = 2.f;

	running = true;
}

void AppEngine::Cleanup()
{
	std::cout << "Cleaning up AppEngine." << std::endl;
	
	for (size_t i = 0; i < states.size(); i++)
		PopState();

	window->close();
	delete window;

	Download clearCache;
	clearCache.clearCache();

	std::cout << "AppEngine cleaned up." << std::endl;
}

void AppEngine::ChangeState(AppState* state)
{
	queuedEvents.push_back(std::pair<EventType, AppState*>(EventType::ChangeState, state));
}

void AppEngine::PushState(AppState* state)
{
	queuedEvents.push_back(std::pair<EventType, AppState*>(EventType::PushState, state));
}

void AppEngine::PopState()
{
	queuedEvents.push_back(std::pair<EventType, AppState*>(EventType::PopState, nullptr));
}

void AppEngine::HandleEvents()
{
	if (running && !states.empty())
		states.back()->HandleEvents();

	for (size_t i = 0; i < queuedEvents.size(); i++)
	{
		if (queuedEvents[i].first == EventType::ChangeState)
		{
			if (!states.empty())
			{
				states.back()->Cleanup();

				delete states.back();
				states.pop_back();
			}

			states.push_back(queuedEvents[i].second);
			states.back()->Init(this);
		}
		else if (queuedEvents[i].first == EventType::PushState)
		{
			if (!states.empty())
				states.back()->Pause();

			states.push_back(queuedEvents[i].second);
			states.back()->Init(this);
		}
		else if (queuedEvents[i].first == EventType::PopState)
		{
			if (!states.empty())
			{
				states.back()->Cleanup();

				delete states.back();
				states.pop_back();
			}

			if (!states.empty())
				states.back()->Resume();
		}
		else if (queuedEvents[i].first == EventType::Quit)
		{
			// TODO: implement bobwars appengine quit
		}

		// this might break things, but I'm not sure.
		// I am confident it will work.
		queuedEvents.pop_back();
	}
}

void AppEngine::Update()
{
	if (running)
		states.back()->Update();
}

void AppEngine::Draw()
{
	if (running)
		states.back()->Draw();
}

void AppEngine::UpdateViewSize(const sf::Vector2f& size)
{
	std::cout << "new width: " << size.x << std::endl;
	std::cout << "new height: " << size.y << std::endl;

	sf::Vector2u newSize(size.x, size.y);

	//TODO: define max size somewhere
	if (newSize.x >= 525 && newSize.y >= 325)
	{
		sf::FloatRect visibleArea(0, 0, size.x, size.y);
		window->setView(sf::View(visibleArea));
	}
	else
	{
		if (size.x <= settings.window.width)
			newSize.x = settings.window.width;

		if (size.x <= settings.window.height)
			newSize.y = settings.window.height;

		window->setSize(newSize);
	}

	SetMultiThreadedIndicatorPosition(sf::Vector2f(20, window->getSize().y - 20));
}

void AppEngine::ShowMultiThreadedIndicator()
{
	multithreaded_process_indicator.rotate(1);
	window->draw(multithreaded_process_indicator);
}

void AppEngine::SetMultiThreadedIndicatorPosition(const sf::Vector2f& pos)
{
	multithreaded_process_indicator.setPosition(pos);
}

void AppEngine::LoadMultiThreadedIcon(std::string iconPosition)
{
	multithreaded_process_indicator_tex.loadFromFile(iconPosition);
}