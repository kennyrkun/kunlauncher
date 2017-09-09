#include "AppEngine.hpp"
#include "AppState.hpp"

#include <iostream>

void AppEngine::Init(std::string title, int width, int height, bool fullscreen)
{
	// initialize SDL
	window = new sf::RenderWindow(sf::VideoMode(width, height), "AppEngineWindow");
	window->setVerticalSyncEnabled(true);

	m_fullscreen = fullscreen;
	m_running = true;

	std::cout << "AppEngine Init" << "\n";
}

void AppEngine::Cleanup()
{
	std::cout << "Cleaning up AppEngine." << "\n";
	
	// cleanup the all states
	while (!states.empty())
	{
		states.back()->Cleanup();
		states.pop_back();
	}

	window->close();

	std::cout << "AppEngine cleaned up." << "\n";
}

void AppEngine::ChangeState(AppState* state)
{
	// cleanup the current state
	if (!states.empty()) {
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
	{
		states.back()->Resume();
	}
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
