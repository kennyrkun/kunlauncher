#include "AppEngine.hpp"
#include "AppState.hpp"
#include "InitialiseState.hpp"
#include "HomeState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "LauncherUpdater.hpp"
#include "ProgressBar.hpp"
#include "SettingsParser.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>

InitialiseState InitialiseState::IntialiseState_dontfuckwithme;

namespace fs = std::experimental::filesystem;

void InitialiseState::Init(AppEngine* app_)
{
	std::cout << "IntialiseState Init" << std::endl;

	app = app_;

	if (!font.loadFromFile(".\\" + GBL::DIR::BASE + "\\" + GBL::DIR::RESOURCE + "\\" + GBL::DIR::FONT + "\\Product Sans.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << std::endl;

		if (!font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font!" << std::endl;

			abort();
		}
	}

	app->window->create(sf::VideoMode(400, 150), app->title, sf::Style::None);

	app->window->setVerticalSyncEnabled(app->settings.verticalSync);
	app->window->setTitle("KunLauncher " + GBL::VERSION);

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
		app->developerModeActive = true;
	}

	sf::RenderWindow* newWindow = new sf::RenderWindow(sf::VideoMode(app->settings.width, app->settings.height), "KunLauncher " + GBL::VERSION, sf::Style::Resize | sf::Style::Close);
	newWindow->setVerticalSyncEnabled(true);

	app->window->close();
	delete app->window;
	app->window = nullptr;
	app->window = newWindow;

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
			app->developerModeActive = true;
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
		std::cout << "helper is done, joining" << std::endl;

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

	app->window->clear(GBL::COLOR::BACKGROUND);

	app->window->draw(initialiseText);
	app->window->draw(currentLauncherTask);
	app->window->draw(thingthatdoesntripoffvisualstudio);
	app->window->draw(*progressBar);

	app->window->display();
}

void InitialiseState::initialisise()
{
	setTaskText("initialising");

	validateFileStructure();

	progressBar->addThingToDo();
	{
		std::cout << "loading config" << std::endl;
		SettingsParser settings;
		if (settings.loadFromFile(".\\" + GBL::DIR::BASE + "kunlauncher.conf"))
		{
			settings.get("window_width", app->settings.width);
			settings.get("window_height", app->settings.height);
			settings.get("updatelauncheronstart", app->settings.updateLauncherOnStart);
			settings.get("checkforitemsonstart", app->settings.checkForNewItemsOnStart);
			settings.get("experimentalThemes", app->settings.experimentalThemes);
			settings.get("defaultTheme", app->settings.theme);
		}
		else
		{
			std::cout << "failed to load settings, using defaults" << std::endl;
		}
	}
	progressBar->oneThingDone(); // 3

	if (app->settings.checkForNewItemsOnStart)
	{
		std::cout << "updating app index." << std::endl;
		progressBar->addThingsToDo(2);

		Download getItemIndex;
		getItemIndex.setInputPath("./" + GBL::DIR::WEB_APP_DIRECTORY + "/index.dat");
		getItemIndex.setOutputDir(".\\" + GBL::DIR::BASE + GBL::DIR::APPS);
		getItemIndex.setOutputFilename("\\index.dat");

		getItemIndex.download();
		progressBar->oneThingDone();
		getItemIndex.save();
		progressBar->oneThingDone();
	}
	else
	{
		std::cout << "skipping item update" << std::endl;
	}

	if (app->settings.updateLauncherOnStart)
	{
		setTaskText("checking for updates");
		progressBar->addThingToDo(); // check for updates

		updater = new LauncherUpdater;
		if (updater->checkForUpdates())
		{
			progressBar->oneThingDone(); // check for update

			MessageBox::Options modOptions;

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
				std::cout << "yes, update now." << std::endl;
				progressBar->addThingsToDo(2); // update and replace exe

				setTaskText("downloading update");
				updater->downloadUpdate();
				progressBar->oneThingDone(); // update

				setTaskText("replacing old executable");
				updater->replaceOldExecutable();
				progressBar->oneThingDone(); // replace exe

				MessageBox::Options modOptions;
				modOptions.text = "Launcher updated";

				modOptions.text = "Launcher updated to v" + updater->remoteVersion + "! Restart it!";
				modOptions.settings = { "Restart Now", "Restart Later" };

				MessageBox updateSuccessfulModal(modOptions);
				updateSuccessfulModal.runBlocking();

				switch (updateSuccessfulModal.returnCode)
				{
				case 0:
				{
					std::cout << "restarting now" << std::endl;
					restartNow = true;
					break;
				}

				case 1:
				{
					std::cout << "restarting later" << std::endl;
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
				std::cout << "don't update now" << std::endl;
				break;
			}

			default:
				break;
			}
		}
	}
	else
	{
		std::cout << "skipping check for updates" << std::endl;
	}

	getThemeConfiguration();

	sf::sleep(sf::seconds(2));

	setTaskText("ready");

	helperDone = true;
}

int InitialiseState::validateFileStructure()
{
	setTaskText("validating files");

	progressBar->addThingsToDo(5); // bin, config, apps, app index, resources

	std::cout << "checking for bin" << std::endl;
	if (!fs::exists(".\\" + GBL::DIR::BASE)) // 1
	{
		std::cout << "bin folder missing, creating" << std::endl;
		progressBar->addThingToDo();

		fs::create_directory(".\\" + GBL::DIR::BASE);

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 1

	std::cout << "checking for config" << std::endl;
	if (!fs::exists(".\\" + GBL::DIR::BASE + "kunlauncher.conf")) // 2
	{
		std::cout << "config file missing, creating" << std::endl;
		progressBar->addThingToDo();

		std::ofstream createConfigurationFile(".\\" + GBL::DIR::BASE + "kunlauncher.conf");

		createConfigurationFile << "// default configuration for kunlauncher" << std::endl;
		createConfigurationFile << std::endl;
		createConfigurationFile << "window_width = 525" << std::endl;
		createConfigurationFile << "window_height = 400" << std::endl;
		createConfigurationFile << "updatelauncheronstart = TRUE" << std::endl;
		createConfigurationFile << "checkfornewitemsonstart = TRUE" << std::endl;
		createConfigurationFile << "experimentalThemes = FALSE" << std::endl;

		createConfigurationFile.close();

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 2

	std::cout << "checking for third party notices" << std::endl;
	if (!fs::exists(".\\" + GBL::DIR::BASE + "thirdpartynotices.txt")) // 4
	{
		std::cout << "missing thirdpartynotices, downloading..." << std::endl;
		progressBar->addThingToDo();

		Download getThirdPartyNotices;
		getThirdPartyNotices.setInputPath("./" + GBL::DIR::WEB_DIRECTORY + "/thirdpartynotices.txt");
		getThirdPartyNotices.setOutputDir(".\\" + GBL::DIR::BASE);
		getThirdPartyNotices.setOutputFilename("\\thirdpartynotices.txt");
		getThirdPartyNotices.download();

		// TODO: update this if we already have it

		switch (getThirdPartyNotices.htmlReturnCode)
		{
		case sf::Http::Response::Status::Ok:
		{
			getThirdPartyNotices.save();

			std::cout << "successfully downloaded and saved third party notices." << std::endl;
		}
		case sf::Http::Response::Status::NotFound:
		{
			std::cout << "failed to download third party notices." << std::endl;
		}
		default:
			break;
		}
	}
	progressBar->oneThingDone(); // 3

	std::cout << "checking for apps" << std::endl;
	if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::APPS)) // 5
	{
		std::cout << GBL::DIR::BASE + GBL::DIR::APPS << std::endl;

		std::cout << "apps folder missing, creating" << std::endl;
		progressBar->addThingToDo();

		fs::create_directory(".\\" + GBL::DIR::BASE + GBL::DIR::APPS);

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 4

	std::cout << "checking for apps+index" << std::endl;
	if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::APPS + "index.dat")) // 6
	{
		std::cout << "app index missing, creating" << std::endl;
		progressBar->addThingToDo();

		Download getItemIndex;
		getItemIndex.setInputPath("./" + GBL::DIR::WEB_APP_DIRECTORY + "/index.dat");
		getItemIndex.setOutputDir(".\\" + GBL::DIR::BASE + GBL::DIR::APPS);
		getItemIndex.setOutputFilename("\\index.dat");
		getItemIndex.download();
		getItemIndex.save();

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 5

	return 0;
}

int InitialiseState::updateFileStructure()
{
	return 0;
}

int InitialiseState::getThemeConfiguration()
{
	if (app->settings.experimentalThemes) // 8
	{
		std::cout << "checking for theme file" << std::endl;
		progressBar->addThingToDo();

		if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "dark.sfuitheme"))
		{
			progressBar->addThingToDo();
			std::cout << "creating theme file" << std::endl;
			std::ofstream createDarkTheme(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "dark.sfuitheme");

			createDarkTheme << "// default 'dark' theme for kunlauncher" << std::endl;
			createDarkTheme << std::endl;
			createDarkTheme << "// globals" << std::endl;
			createDarkTheme << "global_background = 50, 50, 50" << std::endl;
			createDarkTheme << "global_text = 255, 255, 255" << std::endl;
			createDarkTheme << std::endl;
			createDarkTheme << "// scrollbar (useless because SFUI currently does not support themes)" << std::endl;
			createDarkTheme << "//scrollbar_scrollbar = 80, 80, 80" << std::endl;
			createDarkTheme << "//scrollbar_scrollthumb = 110, 110, 110" << std::endl;
			createDarkTheme << "//scrollbar_scrollthumb_hover = 158, 158, 158" << std::endl;
			createDarkTheme << "//scrollbar_scrollthumb_hold = 239, 235, 339" << std::endl;
			createDarkTheme << std::endl;
			createDarkTheme << "// items" << std::endl;
			createDarkTheme << "item_card = 100, 100, 100" << std::endl;
			createDarkTheme << "item_icon = 255, 255, 255" << std::endl;
			createDarkTheme << "item_icon_hover = 255, 255, 255" << std::endl;
			createDarkTheme << "item_redownload = 255, 255, 255" << std::endl;
			createDarkTheme << "item_update_is_available = 255, 200, 0" << std::endl;
			createDarkTheme << "item_download = 255, 255, 255" << std::endl;
			createDarkTheme << std::endl;
			createDarkTheme << "// links" << std::endl;
			createDarkTheme << "link_card = 100, 100, 100" << std::endl;
			createDarkTheme << "link_follow = 255, 200, 0" << std::endl;
			createDarkTheme << "link_text = 0, 170, 232" << std::endl;

			createDarkTheme.close();
		}

		if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "light.sfuitheme"))
		{
			progressBar->addThingToDo();
			std::cout << "creating theme file" << std::endl;
			std::ofstream createLightTheme(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "light.sfuitheme");

			createLightTheme << "// default 'light' theme for kunlauncher" << std::endl;
			createLightTheme << std::endl;
			createLightTheme << "// globals" << std::endl;
			createLightTheme << "global_background = 255, 255, 255" << std::endl;
			createLightTheme << "global_text = 0, 0, 0" << std::endl;
			createLightTheme << std::endl;
			createLightTheme << "// scrollbar (useless because SFUI currently does not support themes)" << std::endl;
			createLightTheme << "//scrollbar_scrollbar = 80, 80, 80" << std::endl;
			createLightTheme << "//scrollbar_scrollthumb = 110, 110, 110" << std::endl;
			createLightTheme << "//scrollbar_scrollthumb_hover = 158, 158, 158" << std::endl;
			createLightTheme << "//scrollbar_scrollthumb_hold = 239, 235, 339" << std::endl;
			createLightTheme << std::endl;
			createLightTheme << "// items" << std::endl;
			createLightTheme << "item_card = 192, 192, 192" << std::endl;
			createLightTheme << "item_icon = 255, 255, 255" << std::endl;
			createLightTheme << "item_icon_hover = 255, 255, 255" << std::endl;
			createLightTheme << "item_redownload = 255, 255, 255" << std::endl;
			createLightTheme << "item_update_is_available = 255, 200, 0" << std::endl;
			createLightTheme << "item_download = 255, 255, 255" << std::endl;
			createLightTheme << std::endl;
			createLightTheme << "// links" << std::endl;
			createLightTheme << "link_card = 100, 100, 100" << std::endl;
			createLightTheme << "link_follow = 255, 200, 0" << std::endl;
			createLightTheme << "link_text = 0, 170, 232" << std::endl;

			createLightTheme.close();
		}

		std::cout << "loading theme settings" << std::endl;

		SettingsParser settings;
		if (settings.loadFromFile(".\\" + GBL::DIR::BASE + "kunlauncher.conf"))
			settings.get("defaultTheme", app->settings.theme);
		std::cout << app->settings.theme << std::endl;

		if (settings.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + app->settings.theme + ".sfuitheme"))
		{
			std::cout << "loaded theme \"" << app->settings.theme << "\"." << std::endl;

			// globals
			std::vector<int> colors;

			if (settings.get("global_background", colors))
			{
				GBL::COLOR::BACKGROUND = sf::Color(colors[0], colors[1], colors[2]);
				colors.clear();
			}

			if (settings.get("global_text", colors))
			{
				GBL::COLOR::TEXT = sf::Color(colors[0], colors[1], colors[2]);
				colors.clear();
			}

			/* scrollbar
			std::cout << "scrollbar theme settings" << std::endl;

			settings.get("scrollbar_scrollbar", colors);
			GBL::COLOR::SCROLLBAR::SCROLLBAR = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();

			settings.get("scrollbar_scrollthumb", colors);
			GBL::COLOR::SCROLLBAR::SCROLLTHUMB = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();

			settings.get("scrollbar_scrollthumb_hover", colors);
			GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();

			settings.get("scrollbar_scrollthumb_hold", colors);
			GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOLD = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear(); */

			// items
			std::cout << "item theme settings" << std::endl;

			if (settings.get("item_card", colors))
			{
				GBL::COLOR::ITEM::CARD = sf::Color(colors[0], colors[1], colors[2]);
				colors.clear();
			}

			if (settings.get("item_icon", colors))
			{
				GBL::COLOR::ITEM::ICON = sf::Color(colors[0], colors[1], colors[2]);
				colors.clear();
			}

			if (settings.get("item_redownload", colors))
			{
				GBL::COLOR::ITEM::REDOWLOAD = sf::Color(colors[0], colors[1], colors[2]);
				colors.clear();
			}

			if (settings.get("item_update_is_available", colors))
			{
				GBL::COLOR::ITEM::UPDATE_IS_AVAILABLE = sf::Color(colors[0], colors[1], colors[2]);
				colors.clear();
			}

			if (settings.get("item_download", colors))
			{
				GBL::COLOR::ITEM::DOWNLOAD = sf::Color(colors[0], colors[1], colors[2]);
				colors.clear();
			}

			std::cout << "done applying themes" << std::endl;
		}
		else
		{
			std::cout << "failed to load settings file" << std::endl; // use default colours
		}

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 8

	return 0;
}

void InitialiseState::setTaskText(std::string text)
{
	std::cout << "TASK: " << text << std::endl;
	currentLauncherTask.setString(text);

	if (currentLauncherTask.getLocalBounds().width > app->window->getSize().x)
	{
		std::cout << "that's some long text right there!" << std::endl;
	}

	currentLauncherTask.setOrigin(currentLauncherTask.getLocalBounds().width / 2, currentLauncherTask.getLocalBounds().height - 20);
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(initialiseText.getPosition().y + 50)));
}

// move theme stuff out of validateFIleStructure
// that function should only check for files and create them if they do not exist
