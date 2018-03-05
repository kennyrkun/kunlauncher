#include "AppEngine.hpp"
#include "AppState.hpp"

#include "Globals.hpp"
#include "Download.hpp"

#include <iostream>

void AppEngine::Init(std::string title_, AppSettings settings_)
{
	settings = settings_;

	window = new sf::RenderWindow;
	window->setVerticalSyncEnabled(settings.verticalSync);
	window->setKeyRepeatEnabled(false);
	running = true;

	multithreaded_process_indicator_tex.loadFromFile(GBL::DIR::textures + "settings_2x.png");
	multithreaded_process_indicator.setRadius(20);
	multithreaded_process_indicator.setOrigin(sf::Vector2f(20, 20));
	multithreaded_process_indicator.setTexture(&multithreaded_process_indicator_tex);

	std::cout << "AppEngine Init" << std::endl;
}

void AppEngine::Cleanup()
{
	std::cout << "Cleaning up AppEngine." << std::endl;
	
	// cleanup the all states
	while (!states.empty())
	{
		states.back()->Cleanup();
		states.pop_back();
	}

	window->close();
	delete window;

	Download clearCache;
	clearCache.clearCache();

	std::cin.get();

	std::cout << "AppEngine cleaned up." << std::endl;
}

void AppEngine::ChangeState(AppState* state)
{
	if (!states.empty()) 
	{
		states.back()->Cleanup();
		states.pop_back();
	}

	// store and init the new state
	states.push_back(state);
	states.back()->Init(this);
}

void AppEngine::PushState(AppState* state)
{
	// pause current state
	if (!states.empty())
		states.back()->Pause();

	// store and init the new state
	states.push_back(state);
	states.back()->Init(this);
}

void AppEngine::PopState()
{
	// cleanup the current state
	if (!states.empty())
	{
		states.back()->Cleanup();
		states.pop_back();
	}

	// resume previous state
	if (!states.empty())
		states.back()->Resume();
}

void AppEngine::HandleEvents()
{
	states.back()->HandleEvents();
}

void AppEngine::Update()
{
	// let the state update the game
	states.back()->Update();
}

void AppEngine::Draw()
{
	// let the state draw the screen
	states.back()->Draw();
}

void AppEngine::UpdateViewSize(const sf::Vector2f & size)
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
		if (size.x <= settings.width)
			newSize.x = settings.width;

		if (size.x <= settings.height)
			newSize.y = settings.height;

		window->setSize(newSize);
	}

	SetMultiThreadedIndicatorPosition(sf::Vector2f(20, window->getSize().y - 20));
}

void AppEngine::ShowMultiThreadedIndicator()
{
	multithreaded_process_indicator.rotate(1);
	window->draw(multithreaded_process_indicator);
}

void AppEngine::SetMultiThreadedIndicatorPosition(const sf::Vector2f & pos)
{
	multithreaded_process_indicator.setPosition(pos);
}
