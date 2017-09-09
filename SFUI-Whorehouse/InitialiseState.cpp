#include "AppEngine.hpp"
#include "AppState.hpp"
#include "InitialiseState.hpp"
#include "HomeState.hpp"

#include "json.hpp"
#include "Download.hpp"
#include "Modal.hpp"
#include "Item.hpp"
#include "Link.hpp"
#include "Globals.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>
#include <mutex>

InitialiseState InitialiseState::IntialiseState_dontfuckwithme;

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

	app->window->setTitle("KunLauncher " + CONST::VERSION);

	homeText.setFont(font);
	homeText.setCharacterSize(72);
	homeText.setString("initialising");
	homeText.setOrigin(homeText.getLocalBounds().width / 2, homeText.getLocalBounds().height - 20);
	homeText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y)));

	currentLauncherTask.setFont(font);
	currentLauncherTask.setCharacterSize(26);
	setTaskText("waiting");

	currentLauncherSubtask.setFont(font);
	currentLauncherSubtask.setCharacterSize(15);
	setTaskSubtext("waiting");

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
		developerActivated = true;
	}

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

void InitialiseState::HandleEvents()
{
	sf::Event event;
	while (app->window->pollEvent(event))
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

		app->ChangeState(HomeState::Instance());

		return;
	}

	app->window->clear(CONST::COLOR::BACKGROUND);

	app->window->draw(homeText);
	app->window->draw(currentLauncherTask);
	app->window->draw(currentLauncherSubtask);

	app->window->display();
}

void InitialiseState::initialisise()
{
	if (!std::experimental::filesystem::exists(".\\" + CONST::DIR::BASE))
	{
		setTaskSubtext("creating bin folder");
		std::experimental::filesystem::create_directory(".\\" + CONST::DIR::BASE);

		settings.updateItemsOnStart = true;
	}
	else
	{
		if (std::experimental::filesystem::exists(".\\" + CONST::DIR::BASE + CONST::DIR::RESOURCE + CONST::DIR::TEXTURE + "icon.png"))
		{
			//	sf::Image icon;
			//	icon.loadFromFile(".\\" + BASE_FOLDER + "\\res\\tex\\icon.png");
			//	window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
		}
	}

	if (!std::experimental::filesystem::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS))
	{
		setTaskSubtext("checking if apps folder exists");

		setTaskSubtext("creating apps folder");
		std::experimental::filesystem::create_directory(".\\" + CONST::DIR::BASE + "apps");

		settings.updateItemsOnStart = true;
	}

	setTaskSubtext("checking if index file exists");
	if (!std::experimental::filesystem::exists(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat"))
	{
		setTaskSubtext("creating empty index files");
		std::ofstream createIndex(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat");
		createIndex.close();

		settings.updateItemsOnStart = true;
	}

	//TODO: this will stop items from updating if it's disabled
	if (settings.updateItemsOnStart)
	{
		setTaskSubtext("checking for item updates");

		// download the index file (or at least store it)
		sf::Http http(CONST::DIR::WEB_HOSTNAME);
		sf::Http::Request request("./" + CONST::DIR::WEB_APP_DIRECTORY + "/index.dat", sf::Http::Request::Get);
		sf::Http::Response response = http.sendRequest(request);

		int fileSize = response.getBody().size();

		// if the index file on the server has a different filesize than the one we've got, download it
		setTaskSubtext("gathering apps list");
		if (std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") != fileSize)
		{
			std::cout << "index file has been updated (difference of ";
			if (std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") > fileSize)
			{
				std::cout << std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") - fileSize << " bytes)" << "\n";
			}
			else
			{
				std::cout << fileSize - std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + CONST::DIR::APPS + "index.dat") << " bytes)" << "\n";
			}

			setTaskSubtext("updating apps list");
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

	if (settings.updateLauncherOnStart)
	{
		setTaskText("checking for item updates...");

		if (checkForLauncherUpdates())
		{
			bool doUpdate = false;

			Download getHoHouse;
			getHoHouse.setInputPath("version.info");
			getHoHouse.download();

			if (getHoHouse.fileBuffer != CONST::VERSION)
			{
				ModalOptions modOptions;

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

				setTaskText("waiting on user confirmation");
				setTaskSubtext("you want it sHI-NEY like the treasure from a sunken pirate wreck??");
				Modal doYouWannaUpdate(modOptions);

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

				ModalOptions modOptions;
				modOptions.text = "Launcher updated";

				if (remoteVersion.find("500 Internal Server Error") != std::string::npos)
				{
					modOptions.title = "BRKOEN AGAIN!";
					modOptions.text = "server fucked up, try again.";
				}
				else
				{
					modOptions.text = "Launcher updated to v" + remoteVersion + "! Restart it!";
					modOptions.settings = { "Restart Now", "Restart Later", "View Changelog" };
				}

				Modal updateSuccessfulModal(modOptions);

				switch (updateSuccessfulModal.returnCode)
				{
				case 0:
					std::cout << "restarting now" << "\n";
					exit(0); // TODO: shutdown properly
					break;

				case 1:
					std::cout << "restarting later" << "\n";
					updateSuccessfulModal.close();
					break;

				case 2:
				{
#if defined (_WIN32) // one day it'll be cross platform... one day.
					std::cout << "opening changelog" << "\n";
					std::string command("start " + CONST::DIR::BASE + "\\change.log");
					system(command.c_str());
#else
					std::cout << "This function is not supported on your platform!" << "\n";
#endif
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

	setTaskText("ready");

	helperDone = true;
}

bool InitialiseState::checkForLauncherUpdates()
{
	setTaskText("checking for launcher updates...");

	setTaskSubtext("checking for old launcher executable");
	if (std::experimental::filesystem::exists("kunlauncher.exe.old"))
	{
		try
		{
			setTaskSubtext("removing old executable");
			std::experimental::filesystem::remove("kunlauncher.exe.old");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << "\n";
		}
	}

	setTaskSubtext("retrieving public launcher version");
	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	std::string remoteVersion = getHoHouse.fileBuffer;

	std::cout << "r" << remoteVersion << " : " << "l" << CONST::VERSION << "\n";

	if (remoteVersion != CONST::VERSION)
	{
		std::cout << "launcher is out of date" << "\n";
		return true;
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
	setTaskSubtext("downloading updated launcher");
	getNewWhorehouse.download();

	try
	{
		setTaskSubtext("removing old changelog");
		std::experimental::filesystem::remove(CONST::DIR::BASE + "\\change.log");

		Download getChangelog;
		getChangelog.setInputPath(CONST::DIR::BASE + "\\change.log");
		getChangelog.setOutputDir(".\\");
		getChangelog.setOutputFilename(CONST::DIR::BASE + "\\change.log");
		setTaskSubtext("downloading new changelog");
		getChangelog.download();
		setTaskSubtext("saving new changelog");
		getChangelog.save();
	}
	catch (const std::exception& e)
	{
		std::cout << "unable to download new changelog :" << "\n";
		std::cout << e.what() << "\n";
	}

	try
	{
		setTaskSubtext("replacing old launcher");
		std::experimental::filesystem::rename("kunlauncher.exe", "kunlauncher.exe.old");

		setTaskSubtext("saving updated launcher");
		getNewWhorehouse.save();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << "\n";
		abort();
	}

	return newVersion;
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
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y + 70)));
}

void InitialiseState::setTaskSubtext(std::string text)
{
	std::cout << text << "\n";
	currentLauncherSubtask.setString(text);
	currentLauncherSubtask.setOrigin(currentLauncherSubtask.getLocalBounds().width / 2.0f, currentLauncherSubtask.getLocalBounds().height - 20.0f);
	currentLauncherSubtask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y + 96)));
}
