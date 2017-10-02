#include "AppEngine.hpp"
#include "AppState.hpp"
#include "InitialiseState.hpp"
#include "HomeState.hpp"

#include "json.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "Item.hpp"
#include "Link.hpp"
#include "ProgressBar.hpp"
#include "Globals.hpp"

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
	progressBar->thingsToDo = 5;
	progressBar->thingsDone = 0;

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
//	sf::Event event;
//	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
		}
		else if (event.type == sf::Event::EventType::Resized)
		{
			std::cout << "new window width: " << event.size.width << "\n";
			std::cout << "new window height: " << event.size.height << "\n";

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::View newView = sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height));
				app->window->setView(newView);

				newView.setSize(event.size.width, event.size.height);
				newView.setCenter(newView.getSize().x / 2, newView.getSize().y / 2);
			}
			else
			{
				if (event.size.width <= 525)
					newSize.x = 525;

				if (event.size.height <= 325)
					newSize.y = 325;

				app->window->setSize(newSize);
			}
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
			exit(0);
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
	setTaskText("validating files");

	if (!fs::exists(".\\" + CONST::DIR::BASE))
	{
		std::cout << "bin folder does not exist, creating it" << std::endl;

		fs::create_directory(".\\" + CONST::DIR::BASE);

		app->settings.updateItemsOnStart = true;
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
	progressBar->oneThingDone(); // task 1

	if (!fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS))
	{
		std::cout << "apps folder does not exist, creating it" << std::endl;

		fs::create_directory(".\\" + CONST::DIR::BASE + "apps");

		app->settings.updateItemsOnStart = true;
	}
	progressBar->oneThingDone(); // task 2

	if (!fs::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat"))
	{
		std::cout << "app index file does not exist, creating one" << std::endl;

		std::ofstream createIndex(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat");
		createIndex.close();

		app->settings.updateItemsOnStart = true;
	}
	progressBar->oneThingDone(); // task 3

	//TODO: this will stop items from updating if it's disabled
	if (app->settings.updateItemsOnStart)
	{
		// download the index file (or at least store it)
		sf::Http http(CONST::DIR::WEB_HOSTNAME);
		sf::Http::Request request("./" + CONST::DIR::WEB_APP_DIRECTORY + "/index.dat", sf::Http::Request::Get);
		sf::Http::Response response = http.sendRequest(request);

		int fileSize = response.getBody().size();

		// if the index file on the server has a different filesize than the one we've got, download it
		if (fs::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") != fileSize)
		{
			std::cout << "remote index file is different (difference of ";
			if (fs::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") > fileSize)
			{
				std::cout << fs::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") - fileSize << " bytes)" << "\n";
			}
			else
			{
				std::cout << fileSize - fs::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") << " bytes)" << "\n";
			}

			std::cout << "updating apps list" << "\n";

			std::string fileContainer = response.getBody();

			std::ofstream downloadFile(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat", std::ios::out | std::ios::binary);
			std::cout << "saving file to \"" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat\"... ";

			for (int i = 0; i < fileSize; i++)
				downloadFile << fileContainer[i];
			downloadFile.close();

			if (downloadFile.fail())
				std::cout << "failed" << "\n";
			else
				std::cout << "finished" << "\n";

			std::cout << "index file is ready." << "\n";
		}
	}
	progressBar->oneThingDone(); // task 4

	if (app->settings.updateLauncherOnStart)
	{
		setTaskText("checking for launcher updates");

		if (checkForLauncherUpdates())
		{
			bool doUpdate = false;

			Download getHoHouse;
			getHoHouse.setInputPath("version.info");
			getHoHouse.download();

			if (getHoHouse.fileBuffer != CONST::VERSION) // not the proper version
			{
				MessageBoxOptions modOptions;

				if (getHoHouse.fileBuffer.find("500 Internal Server Error") != std::string::npos)
				{
					modOptions.title = "BRKOEN AGAIN!";
					modOptions.text = "server fucked up, try again.";
					modOptions.settings = { "reopen the launcher and hope to god it doesn't break again" };
				}
				else
				{
					modOptions.title = "Update Available";
					modOptions.text = "Version " + getHoHouse.fileBuffer + " is available, would you like to update?";
					modOptions.settings = { "Yes", "No" };
				}

				MessageBox doYouWannaUpdate(modOptions);
				doYouWannaUpdate.runBlocking();

				switch (doYouWannaUpdate.returnCode)
				{
				case 0:
					std::cout << "yes, update now." << "\n";
					doUpdate = true;
					break;

				case 1:
					std::cout << "don't update now" << "\n";
					doUpdate = false;
					break;

				default:
					break;
				}
			}

			if (doUpdate)
			{
				std::string remoteVersion = updateLauncher(); // feels kinda hacky

				std::cout << "launcher updated" << std::endl;

				MessageBoxOptions modOptions;
				modOptions.text = "Launcher updated";

//				if (remoteVersion.find("500 Internal Server Error") != std::string::npos)
//				{
//					std::cout << "broken" << std::endl;

//					modOptions.title = "BRKOEN AGAIN!";
//					modOptions.text = "server fucked up, try again.";
//				}
				//else
				{
					modOptions.text = "Launcher updated to v" + remoteVersion + "! Restart it!";
					modOptions.settings = { "Restart Now", "Restart Later", "View Changelog" };
				}

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
			}
			else
			{
				std::cout << "updating skipped" << "\n";
			}
				}
		else
		{
			std::cout << "no updates were found" << "\n";
		}
	}
	else
	{
		std::cout << "skipping check for updates" << "\n";
	}
	progressBar->oneThingDone(); // task 5

	setTaskText("ready");

	helperDone = true;
}

bool InitialiseState::checkForLauncherUpdates()
{
	setTaskText("checking for launcher updates...");

	if (std::experimental::filesystem::exists("kunlauncher.exe.old"))
	{
		try
		{
			std::experimental::filesystem::remove("kunlauncher.exe.old");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << "\n";
		}
	}

	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	std::string remoteVersion = getHoHouse.fileBuffer;

	std::cout << "r" << remoteVersion << " : " << "l" << CONST::VERSION << "\n";

	if (remoteVersion != CONST::VERSION)
	{
		std::cout << "launcher is out of date" << "\n";
//		return true;
		return false;
	}
	else
	{
		std::cout << "launcher is up to date" << "\n";
		return false;
	}
}

std::string InitialiseState::updateLauncher()
{
	setTaskText("updating launcher");
	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();
	std::string newVersion = getHoHouse.fileBuffer;

	setTaskText("updating launcher (" + newVersion + ")");

	Download getNewWhorehouse;
	getNewWhorehouse.setInputPath("latest.noexe");
	getNewWhorehouse.setOutputDir(".\\");
	getNewWhorehouse.setOutputFilename("kunlauncher.exe");
	getNewWhorehouse.download();

	try
	{
		fs::rename("kunlauncher.exe", "kunlauncher.exe.old");

		getNewWhorehouse.save();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
		abort();

		// TODO: handle this better;
	}

	return getHoHouse.fileBuffer;
}

int InitialiseState::validateFileStructure()
{
	// check for bin
		// create bin
			// check for configuration file
				// create configuration file
	
	// check for resources
		// create resources
			// create resource manifest
				// download resources
			// check for textures
				// create textures
					// download textures
			// check for fonts
				// create fonts
					// download fonts
			// check for binaries
				//create binaries
					// download binaries
			// check for theme file
	
	// check for apps
		// create apps
			// create app manifest

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

int InitialiseState::updateResourceFiles()
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

int generateDefaultResources()
{
	return 0;
}

void InitialiseState::setTaskText(std::string text)
{
	std::cout << text << "\n";
	currentLauncherTask.setString(text);
	currentLauncherTask.setOrigin(currentLauncherTask.getLocalBounds().width / 2, currentLauncherTask.getLocalBounds().height - 20);
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(initialiseText.getPosition().y + 50)));
}
