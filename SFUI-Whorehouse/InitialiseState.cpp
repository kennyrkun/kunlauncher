#include "AppEngine.hpp"
#include "AppState.hpp"
#include "InitialiseState.hpp"
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

InitialiseState InitialiseState::IntialiseState_dontfuckwithme;

namespace fs = std::experimental::filesystem;

void InitialiseState::Init(AppEngine* app_)
{
	std::cout << "IntialiseState Init" << "\n";

	app = app_;

	if (!font.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::FONT + "\\Product Sans.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << "\n";

		if (!font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font!" << "\n";

			abort();
		}
	}

	app->window->create(sf::VideoMode(400, 150), app->title, sf::Style::None);

	app->window->setVerticalSyncEnabled(app->settings.verticalSync);
	app->window->setTitle("KunLauncher " + CONST::VERSION);

	initialiseText.setFont(font);
	initialiseText.setCharacterSize(56);
	initialiseText.setString("KunLauncher");
	initialiseText.setOrigin(initialiseText.getLocalBounds().width / 2, initialiseText.getLocalBounds().height - 20);
	initialiseText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y / 2)));

	thingthatdoesntripoffvisualstudio.setSize(sf::Vector2f(app->window->getSize().x, 20));
	thingthatdoesntripoffvisualstudio.setFillColor(sf::Color(100, 100, 100));
	thingthatdoesntripoffvisualstudio.setPosition(0, (app->window->getSize().y - 20));

	progressBar = new ProgressBar(sf::Vector2f(0, (app->window->getSize().y - 20)), thingthatdoesntripoffvisualstudio.getSize().x, thingthatdoesntripoffvisualstudio.getSize().y);

	currentLauncherTask.setFont(font);
	currentLauncherTask.setCharacterSize(20);
	setTaskText("initialising");

	helperThread = new std::thread(&InitialiseState::initialisise, this);
	helperRunning = true;
	helperRunning = false;
	isReady = false;

	std::cout << "thread launched" << "\n";
}

void InitialiseState::Cleanup()
{
	std::cout << "IntialiseState Cleaningup" << "\n";

	std::cout << "initialising finished" << "\n";
	std::cout << "helperDone: " << helperDone << "\n";
	std::cout << "helperRunning: " << helperRunning << "\n";

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
	{
		std::cout << "developer mode activated" << "\n";
		app->developerModeActive = true;
	}

	app->window->close();

	std::cout << "IntialiseState Cleanedup" << "\n";
}

void InitialiseState::Pause()
{
	printf("IntialiseState Pause\n");
}

void InitialiseState::Resume()
{
	std::cout << "IntialiseState Resume" << "\n";
}

void InitialiseState::HandleEvents(sf::Event& event)
{
	if (event.type == sf::Event::EventType::Closed)
	{
		app->Quit();
	}
	else if (event.type == sf::Event::EventType::KeyPressed)
	{
		if (event.key.code == sf::Keyboard::Key::LShift)
		{
			app->developerModeActive = !app->developerModeActive; // flip
			progressBar->setColor(sf::Color::Red, sf::Color::Green, sf::Color::Magenta, sf::Color::Blue);

			std::cout << "developer mode toggled" << std::endl;
		}
	}
}

void InitialiseState::Update()
{

}

void InitialiseState::Draw()
{
	if (helperDone)
	{
		std::cout << "helper is done, joining" << "\n";

		helperThread->join();
		delete helperThread;
		helperDone = true;
		helperRunning = false;
		isReady = true;

		if (restartNow)
		{
			app->Quit();

			// FIXME: cross platform support that is not shit :D
			#ifdef _WIN32
			std::string thingtodo("start kunlauncher.exe");
			system(thingtodo.c_str());
			#endif // _WIN32

			exit(0);
		}
		else 
			app->ChangeState(HomeState::Instance());

		return;
	}

	app->window->clear(CONST::COLOR::BACKGROUND);

	app->window->draw(initialiseText);
	app->window->draw(currentLauncherTask);
	app->window->draw(thingthatdoesntripoffvisualstudio);
	app->window->draw(*progressBar);

	app->window->display();
}

void InitialiseState::initialisise()
{
	validateFileStructure();

	/*
	if (!fs::exists(".\\" + CONST::DIR::BASE))
	{
		std::cout << "bin folder does not exist, creating it" << "\n";

		fs::create_directory(".\\" + CONST::DIR::BASE);

		app->settings.updateItemIndexOnStart = true;
	}
	else
	{
		if (fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::RESOURCE + CONST::DIR::TEXTURE + "icon.png"))
		{
			//	sf::Image icon;
			//	icon.loadFromFile(".\\" + BASE_FOLDER + "\\res\\tex\\icon.png");
			//	window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
		}
	}
	progressBar->oneThingDone(); // check for bine folder

	if (!fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS))
	{
		std::cout << "apps folder does not exist, creating it" << "\n";

		fs::create_directory(".\\" + CONST::DIR::BASE + "apps");

		app->settings.updateItemIndexOnStart = true;
	}
	progressBar->oneThingDone(); // check for apps folder

	if (!fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat"))
	{
		std::cout << "app index file does not exist, creating one" << "\n";

		std::ofstream createIndex(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat");
		createIndex.close();

		app->settings.updateItemIndexOnStart = true;
	}
	progressBar->oneThingDone(); // check for app index in apps
	*/

	//TODO: this will stop items from updating if it's disabled
	if (app->settings.updateItemIndexOnStart)
	{
		progressBar->addThingsToDo(2);

		Download getItemIndex;
		getItemIndex.setInputPath("./" + CONST::DIR::WEB_APP_DIRECTORY + "/index.dat");
		getItemIndex.setOutputDir(".\\" + CONST::DIR::BASE + CONST::DIR::APPS);
		getItemIndex.setOutputFilename("\\index.dat");

		getItemIndex.download();
		progressBar->oneThingDone();
		getItemIndex.save();
		progressBar->oneThingDone();
	}

	if (app->settings.updateLauncherOnStart)
	{
		progressBar->addThingToDo(); // check for updates

		setTaskText("checking for updates");

		updater = new LauncherUpdater;
		if (updater->checkForUpdates())
		{
			progressBar->oneThingDone(); // check for update

			MessageBoxOptions modOptions;

			//TODO: add erorr handling
			modOptions.title = "Update Available";
			modOptions.text = "Version " + updater->remoteVersion + " is available, would you like to update?";
			modOptions.settings = { "Yes", "No" };

			MessageBox doYouWannaUpdate(modOptions);
			doYouWannaUpdate.runBlocking();
			doYouWannaUpdate.close(); // right after it's done

			switch (doYouWannaUpdate.returnCode)
			{
			case 0:
			{
				std::cout << "yes, update now." << "\n";
				progressBar->addThingsToDo(2); // update and replace exe

				setTaskText("downloading update");
				updater->downloadUpdate();
				progressBar->oneThingDone(); // update

				setTaskText("replacing old executable");
				updater->replaceOldExecutable();
				progressBar->oneThingDone(); // replace exe

				MessageBoxOptions modOptions;
				modOptions.text = "Launcher updated";

				modOptions.text = "Launcher updated to v" + updater->remoteVersion + "! Restart it!";
				modOptions.settings = { "Restart Now", "Restart Later" };

				MessageBox updateSuccessfulModal(modOptions);
				updateSuccessfulModal.runBlocking();

				switch (updateSuccessfulModal.returnCode)
				{
				case 0:
				{
					std::cout << "restarting now" << "\n";
					restartNow = true;
					break;
				}

				case 1:
				{
					std::cout << "restarting later" << "\n";
					updateSuccessfulModal.close();
					break;
				}

				default:
					break;
				}

				break;
			}

			case 1:
			{
				std::cout << "don't update now" << "\n";
				break;
			}

			default:
				break;
			}
		}
	}
	else
	{
		std::cout << "skipping check for updates" << "\n";
	}

	setTaskText("ready");

	helperDone = true;
}

int InitialiseState::validateFileStructure()
{
	setTaskText("validating files");

	progressBar->addThingsToDo(5); // bin, config, apps, app index, resources

	std::cout << "checking for bin" << std::endl;
	if (!fs::exists(".\\" + CONST::DIR::BASE)) // 1
	{
		std::cout << "bin folder missing, creating" << std::endl;
		progressBar->addThingToDo();

		fs::create_directory(".\\" + CONST::DIR::BASE);

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 1

	std::cout << "checking for config" << std::endl;
	if (!fs::exists(".\\" + CONST::DIR::BASE + "kunluncher.cfg")) // 2
	{
		std::cout << "config file missing, creating" << std::endl;
		progressBar->addThingToDo();

		std::ofstream createConfigurationFile(".\\" + CONST::DIR::BASE + "kunlaucher.cfg");
		createConfigurationFile.close();

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 2

	std::cout << "checking for apps" << std::endl;
	if (!fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS)) // 3
	{
		std::cout << CONST::DIR::BASE + CONST::DIR::APPS << std::endl;

		std::cout << "apps folder missing, creating" << std::endl;
		progressBar->addThingToDo();

		fs::create_directory(".\\" + CONST::DIR::BASE + CONST::DIR::APPS);

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 3

	std::cout << "checking for apps+index" << std::endl;
	if (!fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat")) // 4
	{
		std::cout << "app index missing, creating" << std::endl;
		progressBar->addThingToDo();

		std::ofstream createAppIndex(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat");
		createAppIndex.close();

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 4

	std::cout << "checking for resources" << std::endl;
	if (!fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::RESOURCE)) // 5
	{
		progressBar->addThingsToDo(2);
		std::cout << "resources missing, creating" << std::endl;

		fs::create_directory(".\\" + CONST::DIR::BASE + CONST::DIR::RESOURCE);
		progressBar->oneThingDone();

		getResourceFiles();
		progressBar->oneThingDone();
	}
	else
	{
		if (!validateResourceFiles())
			getResourceFiles();
	}
	progressBar->oneThingDone(); // 5

	return 0;
}

int InitialiseState::updateFileStructure()
{
	return 0;
}

int InitialiseState::validateResourceFiles()
{
	// retrieve resource list
	// load it into a string

	// find keyword
		// find open bracket
			// parse the things inside there
				// from first " to last " (or maybe ;?)
					// load the filename into a vector
	// find closing bracket

	// go through vectors and make sure we have the files
		// if not download them

	return 0;
}

int InitialiseState::getResourceFiles()
{
	// retrieve resource list
	// load it into a string

	// find keyword
		// find open bracket
			// parse the things inside there
				// from first " to last " (or maybe ;?)
					// load the filename into a vector
		// find closing bracket

	// go through vectors and make sure we have the files
		// if not download them

	return 0;
}

void InitialiseState::setTaskText(std::string text)
{
	std::cout << "TASK: " << text << "\n";
	currentLauncherTask.setString(text);
	currentLauncherTask.setOrigin(currentLauncherTask.getLocalBounds().width / 2, currentLauncherTask.getLocalBounds().height - 20);
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(initialiseText.getPosition().y + 50)));
}
