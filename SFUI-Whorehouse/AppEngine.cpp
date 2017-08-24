#include "AppEngine.hpp"
#include "AppState.hpp"

#include <iostream>

void AppEngine3::Init(std::string title, int width, int height, bool fullscreen)
{
	// initialize SDL
	window = new sf::RenderWindow(sf::VideoMode(width, height), "oof");

	m_fullscreen = fullscreen;
	m_running = true;

	std::cout << "CGameEngine Init" << std::endl;
}

void AppEngine3::Cleanup()
{
	// cleanup the all states
	while (!states.empty())
	{
		states.back()->Cleanup();
		states.pop_back();
	}

	window->close();

	std::cout << "CGameEngine Cleanup" << std::endl;
}

void AppEngine3::ChangeState(AppState3* state)
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

void AppEngine3::PushState(AppState3* state)
{
	// pause current state
	if (!states.empty())
		states.back()->Pause();

	// store and init the new state
	states.push_back(state);
	states.back()->Init(this);
}

void AppEngine3::PopState()
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

void AppEngine3::HandleEvents()
{
	states.back()->HandleEvents();
}

void AppEngine3::Update()
{
	// let the state update the game
	states.back()->Update();
}

void AppEngine3::Draw()
{
	// let the state draw the screen
	states.back()->Draw();
}
