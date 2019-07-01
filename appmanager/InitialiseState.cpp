#include "../SFUI-Whorehouse/AppEngine.hpp"
#include "InitialiseState.hpp"
#include "HomeState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "../SFUI-Whorehouse/ProgressBar.hpp"

#include <SFUI/Theme.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

void InitialiseState::Init(AppEngine* app_)
{
	initTime.restart();
	std::cout << "IntialiseState Init" << std::endl;

	app = app_;

	if (!font.loadFromFile(GBL::DIR::fonts + "Arial.ttf"))
	{
		std::cout << "failed to load provided arial, falling back to windows Arial!" << std::endl;

		if (!font.loadFromFile("C://Windows//Fonts//Arial.ttf"))
		{
			std::cout << "failed to load a font! (init)" << std::endl;

			abort(); // TODO: handle this more gracefully
		}
	}

	app->window->create(sf::VideoMode(400, 150), GBL::NAME, sf::Style::None);
	app->window->setVerticalSyncEnabled(app->settings.window.verticalSync);
	app->window->setTitle("KunLauncher " + std::to_string(GBL::VERSION) + " initalising");

	initialiseText.setFont(font);
	initialiseText.setCharacterSize(56);
	initialiseText.setString("App Manager");
	initialiseText.setOrigin(initialiseText.getLocalBounds().width / 2, initialiseText.getLocalBounds().height - 20);
	initialiseText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y / 2)));
	initialiseText.setFillColor(SFUI::Theme::input.textColor);

	thingthatdoesntripoffvisualstudio.setSize(sf::Vector2f(app->window->getSize().x, 20));
	thingthatdoesntripoffvisualstudio.setFillColor(sf::Color(100, 100, 100));
	thingthatdoesntripoffvisualstudio.setPosition(sf::Vector2f(0, (app->window->getSize().y - 20)));

	progressBar = new ProgressBar(sf::Vector2f(0, (app->window->getSize().y - 20)), thingthatdoesntripoffvisualstudio.getSize().x, thingthatdoesntripoffvisualstudio.getSize().y);

	currentLauncherTask.setFont(font);
	currentLauncherTask.setCharacterSize(20);

	setTaskText("You can't see me!");

	helperThread = new std::thread(&InitialiseState::initialise, this);
	helperRunning = true;
	helperRunning = false;
	isReady = false;

	std::cout << "thread launched" << std::endl;
}

void InitialiseState::Cleanup()
{
	std::cout << "Cleaning up IntialiseState" << std::endl;

	std::cout << "initialising finished" << std::endl;
	std::cout << "helperDone: " << helperDone << std::endl;
	std::cout << "helperRunning: " << helperRunning << std::endl;

	// sneaky beakily create a new window without updating the taskbar
	sf::RenderWindow* newWindow = new sf::RenderWindow(sf::VideoMode(app->settings.window.width, app->settings.window.height), GBL::NAME + " " + std::to_string(GBL::VERSION), sf::Style::Close);
	newWindow->setVerticalSyncEnabled(app->settings.window.verticalSync);

	app->window->close();
	delete app->window;
	app->window = newWindow;

	newWindow->requestFocus();

	std::cout << "Cleaned up IntialiseState. (" << initTime.getElapsedTime().asSeconds() << "s)" << std::endl;
}

void InitialiseState::Pause()
{
	std::cout << "IntialiseState Paused." << std::endl;
}

void InitialiseState::Resume()
{
	std::cout << "IntialiseState Resumed." << std::endl;
}

void InitialiseState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		// TODO: I don't know if I want it to be closeable in this state
		if (event.type == sf::Event::EventType::Closed)
			app->Quit();
	}
}

void InitialiseState::Update()
{
	if (helperDone)
	{
		std::cout << "helper is done, joining" << std::endl;

		helperThread->join();
		delete helperThread;
		helperDone = true;
		helperRunning = false;
		isReady = true;

		app->ChangeState(new HomeState);
		return;
	}
}

void InitialiseState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	app->window->draw(initialiseText);
	app->window->draw(currentLauncherTask);
	app->window->draw(thingthatdoesntripoffvisualstudio);
	app->window->draw(*progressBar);

	app->window->display();
}

void InitialiseState::initialise()
{
	validateFileStructure();

	setTaskText("ready");

	helperDone = true;
}

int InitialiseState::validateFileStructure()
{
	progressBar->reset();
	progressBar->addThingsToDo(3); // bin, cache, appcache, config, tpn, apps, apps+index, resources, themes, stats
	setTaskText("validating files");

	std::cout << "checking for bin" << std::endl; 
	{ // 1
		if (!fs::exists(GBL::DIR::installDir))
		{
			std::cout << "bin folder missing, creating" << std::endl;
			fs::create_directories(GBL::DIR::installDir);
		}
	} progressBar->oneThingDone();

	std::cout << "checking for textures" << std::endl; 
	{ // 2
		if (!fs::exists(GBL::DIR::textures))
		{
			std::cout << "textures folder missing, creating" << std::endl;
			fs::create_directories(GBL::DIR::textures);
		}
	} progressBar->oneThingDone();

	std::cout << "checking for apps" << std::endl;
	{ // 3
		if (!fs::exists(GBL::DIR::apps))
		{
			std::cout << "apps folder missing, creating" << std::endl;
			fs::create_directories(GBL::DIR::apps);
		}
	} progressBar->oneThingDone();

	return 0;
}

void InitialiseState::setTaskText(std::string text)
{
	std::cout << "TASK: " << text << std::endl;
	currentLauncherTask.setString(text);

	if (currentLauncherTask.getLocalBounds().width > app->window->getSize().x)
	{
		std::cout << "that's some long text right there!" << std::endl;

		//TODO: text scaling

		//		currentLauncherTask.setCharacterSize(currentLauncherTask.getCharacterSize() / 2);
	}
	else
	{
		currentLauncherTask.setCharacterSize(20);
	}

	currentLauncherTask.setOrigin(sf::Vector2f(static_cast<int>(currentLauncherTask.getLocalBounds().width / 2), static_cast<int>(currentLauncherTask.getLocalBounds().height - 20)));
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(initialiseText.getPosition().y + 50.0f)));
}

// TODO: move theme stuff out of validateFileStructure
// that function should only check for files and create them if they do not exist
