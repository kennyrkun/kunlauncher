#include "AppEngine.hpp"
#include "AppState.hpp"
#include "InitialiseState.hpp"
#include "HomeState.hpp"

#include "Download.hpp"
#include "Modal.hpp"
#include "Item.hpp"
#include "Link.hpp"
#include "constants.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>
#include <mutex>

InitialiseState InitialiseState::IntialiseState_dontfuckwithme;

void InitialiseState::Init(AppEngine* app_)
{
	std::cout << "IntialiseState Init" << std::endl;

	app = app_;

	if (!font.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::FONT + "\\Product Sans.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << std::endl;

		if (!font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font!" << std::endl;

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

	std::cout << "thread launched" << std::endl;
}

void InitialiseState::Cleanup()
{
	std::cout << "IntialiseState Cleaningup" << std::endl;

	std::cout << "initialising finished" << std::endl;
	std::cout << "helperDone: " << helperDone << std::endl;
	std::cout << "helperRunning: " << helperRunning << std::endl;

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift))
	{
		std::cout << "developer mode activated" << std::endl;
		developerActivated = true;
	}

	std::cout << "IntialiseState Cleanedup" << std::endl;
}

void InitialiseState::Pause()
{
	printf("IntialiseState Pause\n");
}

void InitialiseState::Resume()
{
	std::cout << "IntialiseState Resume" << std::endl;
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
			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

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
		std::cout << "helper is done, joining" << std::endl;

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
		if (std::experimental::filesystem::exists(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::TEXTURE + "\\icon.png"))
		{
			//	sf::Image icon;
			//	icon.loadFromFile(".\\" + BASE_FOLDER + "\\res\\tex\\icon.png");
			//	window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
		}
	}

	setTaskSubtext("checking if index file exists");
	if (!std::experimental::filesystem::exists(".\\" + CONST::DIR::BASE + "\\index.dat"))
	{
		setTaskSubtext("creating empty index files");
		std::ofstream createIndex(".\\" + CONST::DIR::BASE + "\\index.dat");
		createIndex.close();

		settings.updateItemsOnStart = true;
	}

	//TODO: this will stop items from updating if it's disabled
	if (settings.updateItemsOnStart)
	{
		setTaskSubtext("checking for item updates");

		// download the index file (or at least store it)
		sf::Http http(CONST::DIR::WEB_HOSTNAME);
		sf::Http::Request request("/index.dat", sf::Http::Request::Get);
		sf::Http::Response response = http.sendRequest(request);

		int fileSize = response.getBody().size();

		// if the index file on the server has a different filesize than the one we've got, download it
		setTaskSubtext("gathering apps list");
		if (std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + "\\index.dat") != fileSize)
		{
			std::cout << "index file has been updated (difference of ";
			if (std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + "\\index.dat") > fileSize)
			{
				std::cout << std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + "\\index.dat") - fileSize << " bytes)" << std::endl;
			}
			else
			{
				std::cout << fileSize - std::experimental::filesystem::file_size(".\\" + CONST::DIR::BASE + "\\index.dat") << " bytes)" << std::endl;
			}

			setTaskSubtext("updating apps list");
			std::cout << "updating apps list" << std::endl;

			std::string fileContainer = response.getBody();
			std::ofstream downloadFile(".\\" + CONST::DIR::BASE + "\\index.dat", std::ios::out | std::ios::binary);
			std::cout << "saving file to \"" + CONST::DIR::BASE + "\\index.dat\"... ";

			for (int i = 0; i < fileSize; i++)
				downloadFile << fileContainer[i];
			downloadFile.close();

			if (downloadFile.fail())
				std::cout << "failed" << std::endl;
			else
				std::cout << "finished" << std::endl;

			std::cout << "index file is ready." << std::endl;
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
					modOptions.title = "fuckin peice of shit is broken again";
					modOptions.text = "server fucked up, committing suicide.";
					modOptions.settings = { "reopen the launcher and hope to god it doesn't break again" };
				}
				else
				{
					modOptions.title = "Update Available";
					modOptions.text = "Version " + getHoHouse.fileBuffer + " is available, would you like to update?";
					modOptions.settings = { "Yes", "No" };
				}

				setTaskText("waiting on user confirmation");
				setTaskSubtext("you want it shiney?");
				Modal doYouWannaUpdate(modOptions);

				switch (doYouWannaUpdate.returnCode)
				{
				case 0:
					std::cout << "yes, update now." << std::endl;
					doUpdate = true;
					break;

				case 1:
					std::cout << "don't update now" << std::endl;
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
					modOptions.text = "server fucked up, committing suicide.";
					modOptions.settings = { "reopen the launcher and hope to god it doesn't break again" };
				}
				else
				{
					modOptions.text = "Launcher updated to v" + remoteVersion + "! Restart it!";
					modOptions.settings = { "Restart Now", "Restart Later" };
				}

				Modal updateSuccessfulModal(modOptions);

				switch (updateSuccessfulModal.returnCode)
				{
				case 0:
					std::cout << "restarting now" << std::endl;
					exit(0); // TODO: shutdown properly
					break;

				case 1:
					std::cout << "restarting later" << std::endl;
					updateSuccessfulModal.close();

				default:
					break;
				}
			}
			else
			{
				std::cout << "updating skipped" << std::endl;
			}
		}
		else
		{
			std::cout << "no updates were found" << std::endl;
		}
	}
	else
	{
		std::cout << "skipping check for updates" << std::endl;
	}

	setTaskText("ready");

	helperDone = true;
}

bool InitialiseState::checkForLauncherUpdates()
{
	setTaskText("checking for updates...");

	if (std::experimental::filesystem::exists("kunlauncher.exe.old"))
	{
		try
		{
			std::experimental::filesystem::remove("kunlauncher.exe.old");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	std::string remoteVersion = getHoHouse.fileBuffer;

	std::cout << "r" << remoteVersion << " : " << "l" << CONST::VERSION << std::endl;

	if (remoteVersion != CONST::VERSION)
		return true;
	else
		return false;
}

std::string InitialiseState::updateLauncher()
{
	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	std::string remoteVersion = getHoHouse.fileBuffer;

	std::cout << "r" << remoteVersion << " : " << "l" << CONST::VERSION << std::endl;

	if (remoteVersion != CONST::VERSION)
	{
		setTaskText("updating launcher");

		std::cout << "launcher is out of date (current: " << CONST::VERSION << "; remote: " << remoteVersion << ")" << std::endl;
		setTaskSubtext("downloading updated launcher");

		Download getNewWhorehouse;
		getNewWhorehouse.setInputPath("latest.noexe");
		getNewWhorehouse.setOutputDir(".\\");
		getNewWhorehouse.setOutputFilename("kunlauncher.exe");
		getNewWhorehouse.download();

		setTaskSubtext("saving updated launcher");

		try
		{
			std::experimental::filesystem::rename("kunlauncher.exe", "kunlauncher.exe.old");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}

		getNewWhorehouse.save();

		return remoteVersion;
	}
	else
	{
		return CONST::VERSION;
	}
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
