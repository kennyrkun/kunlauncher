#include "AppEngine.hpp"
#include "AppState.hpp"

#include <iostream>

#define EOUT(MESSAGE) std::cout << "APPENGINE: " << #MESSAGE << std::endl;

/**
void AppEngine::Init(std::string title, int width, int height)
{
	window = new sf::RenderWindow(sf::VideoMode(width, height), title);
	window->setVerticalSyncEnabled(true);

//	if (fullscreen) 
//	{
//		flags = SDL_FULLSCREEN;
//	}

//	m_fullscreen = fullscreen;
	running = true;

	 std::cout << "ProgramEngine Init" << std::endl;
}

void AppEngine::Cleanup()
{
	// cleanup all the states
	while (!states.empty()) 
	{
		states.back()->Cleanup();
		states.pop_back();
	}

	// switch back to windowed mode so other 
	// programs won't get accidentally resized
	if (fullscreen)
	{
//		screen = SDL_SetVideoMode(640, 480, 0, 0);
	}

	window->close();
	delete window;

	std::cout << "AppEngine Cleanup" << std::endl;
}

void AppEngine::ChangeState(AppState* state)
{
	// cleanup the current state
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
	{
		states.back()->Pause();
	}

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
	// let the state handle events
	states.back()->HandleEvents();
}

void AppEngine::Update()
{
	// let the state update the app
	states.back()->Update();
}

void AppEngine::Draw()
{
	// let the state draw the screen
	states.back()->Draw();
}
*/

// AppEngine2

AppEngine2::AppEngine2(std::string title, int width, int height)
{
	EOUT(initalising AppEngine);

	window = new sf::RenderWindow(sf::VideoMode(width, height), title);
	window->setVerticalSyncEnabled(true);

//	if (fullscreen) 
//	{
//		flags = SDL_FULLSCREEN;
//	}

//	fullscreen = fullscreen;
	running = true;

	std::cout << "ProgramEngine Init" << std::endl;
}

AppEngine2::~AppEngine2()
{
	EOUT(deleting AppEngine);

	delete window;
}

void AppEngine2::ChangeState(AppState2* state)
{
	EOUT(switching states);

	// cleanup the current state
	if (!states.empty())
	{
		std::cout << "doing the delete" << std::endl;

//		delete states.back();
		states.pop_back();

		std::cout << "delete has been done" << std::endl;
	}

	// store and init the new state
	states.push_back(state);

	std::cout << "there are " << states.size() << " states" << std::endl;
}

void AppEngine2::PushState(AppState2* state)
{
	EOUT(pusing state to stack);

	// pause current state
	if (!states.empty())
	{
		EOUT(pausing last state);

		states.back()->Pause();
	}

	// store and init the new state
	states.push_back(state);

	std::cout << "there are " << states.size() << " states" << std::endl;
}

void AppEngine2::PopState()
{
	EOUT(popping back state);

	// cleanup the current state
	if (!states.empty())
	{
		delete states.back();
		states.pop_back();
	}

	// resume previous state
	if (!states.empty())
	{
		EOUT(resuming previous state);

		states.back()->Resume();
	}
	else // no more states
	{
		EOUT(no more states, exiting.);

		Quit();
	}

	std::cout << "there are " << states.size() << " states" << std::endl;
}

void AppEngine2::HandleEvents()
{
	if (running)
	{
		// let the state handle events
		states.back()->HandleEvents();
	}
}

void AppEngine2::Update()
{
	if (running)
	{
		// let the state update the app
		states.back()->Update();
	}
}

void AppEngine2::Draw()
{
	if (running)
	{
		// let the state draw the screen
		states.back()->Draw();
	}
}
