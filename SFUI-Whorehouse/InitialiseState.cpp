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
	std::cout << "IntialiseState Init" << "\n";

	app = app_;

	if (!font.loadFromFile(".\\" + GBL::DIR::BASE + "\\" + GBL::DIR::RESOURCE + "\\" + GBL::DIR::FONT + "\\Product Sans.ttf"))
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

	sf::RenderWindow* newWindow = new sf::RenderWindow(sf::VideoMode(app->settings.width, app->settings.height), "KunLauncher " + GBL::VERSION, sf::Style::Resize | sf::Style::Close);
	newWindow->setVerticalSyncEnabled(true);

	app->window->close();
	delete app->window;
	app->window = nullptr;
	app->window = newWindow;

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
			app->developerModeActive = true;
			progressBar->setColor(sf::Color::Red, sf::Color::Green, sf::Color::Magenta, sf::Color::Blue);

			std::cout << "developer mode toggled" << "\n";
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
		std::cout << "loading config" << "\n";
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
			std::cout << "failed to load settings, using defaults" << "\n";
		}
	}
	progressBar->oneThingDone(); // 3

	if (app->settings.checkForNewItemsOnStart)
	{
		std::cout << "updating app index." << "\n";
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
		std::cout << "skipping item update" << "\n";
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
				std::cout << "yes, update now." << "\n";
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

	getThemeConfiguration();

	sf::sleep(sf::seconds(2));

	setTaskText("ready");

	helperDone = true;
}

int InitialiseState::validateFileStructure()
{
	setTaskText("validating files");

	progressBar->addThingsToDo(6); // bin, config, apps, app index, resources

	std::cout << "checking for bin" << "\n";
	if (!fs::exists(".\\" + GBL::DIR::BASE)) // 1
	{
		std::cout << "bin folder missing, creating" << "\n";
		progressBar->addThingToDo();

		fs::create_directory(".\\" + GBL::DIR::BASE);

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 1

	std::cout << "checking for config" << "\n";
	if (!fs::exists(".\\" + GBL::DIR::BASE + "kunlauncher.conf")) // 2
	{
		std::cout << "config file missing, creating" << "\n";
		progressBar->addThingToDo();

		std::ofstream createConfigurationFile(".\\" + GBL::DIR::BASE + "kunlauncher.conf");

		createConfigurationFile << "// default configuration for kunlauncher" << "\n";
		createConfigurationFile << "\n";
		createConfigurationFile << "window_width = 525" << "\n";
		createConfigurationFile << "window_height = 400" << "\n";
		createConfigurationFile << "updatelauncheronstart = TRUE" << "\n";
		createConfigurationFile << "checkfornewitemsonstart = TRUE" << "\n";
		createConfigurationFile << "experimentalThemes = FALSE" << "\n";

		createConfigurationFile.close();

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 2

	std::cout << "checking for third party notices" << "\n";
	if (!fs::exists(".\\" + GBL::DIR::BASE + "thirdpartynotices.txt")) // 4
	{
		std::cout << "missing thirdpartynotices, downloading..." << "\n";
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

			std::cout << "successfully downloaded and saved third party notices." << "\n";
		}
		case sf::Http::Response::Status::NotFound:
		{
			std::cout << "failed to download third party notices." << "\n";
		}
		default:
			break;
		}
	}
	progressBar->oneThingDone(); // 3

	std::cout << "checking for apps" << "\n";
	if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::APPS)) // 5
	{
		std::cout << GBL::DIR::BASE + GBL::DIR::APPS << "\n";

		std::cout << "apps folder missing, creating" << "\n";
		progressBar->addThingToDo();

		fs::create_directory(".\\" + GBL::DIR::BASE + GBL::DIR::APPS);

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 4

	std::cout << "checking for apps+index" << "\n";
	if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::APPS + "index.dat")) // 6
	{
		std::cout << "app index missing, creating" << "\n";
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

	std::cout << "checking for resources" << "\n";
	if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE)) // 7
	{
		progressBar->addThingsToDo(2);
		std::cout << "resources missing, creating" << "\n";

		fs::create_directory(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE);
		progressBar->oneThingDone();

		getResourceFiles();
		progressBar->oneThingDone();
	}
	else
	{
		if (!validateResourceFiles())
			getResourceFiles();
	}
	progressBar->oneThingDone(); // 6

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
		std::cout << "checking for theme file" << "\n";
		progressBar->addThingToDo();

		if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "dark.sfuitheme"))
		{
			progressBar->addThingToDo();
			std::cout << "creating theme file" << "\n";
			std::ofstream createDarkTheme(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "dark.sfuitheme");

			createDarkTheme << "// default 'dark' theme for kunlauncher" << "\n";
			createDarkTheme << "\n";
			createDarkTheme << "// globals" << "\n";
			createDarkTheme << "global_background = 50, 50, 50" << "\n";
			createDarkTheme << "global_text = 255, 255, 255" << "\n";
			createDarkTheme << "\n";
			createDarkTheme << "// scrollbar (useless because SFUI currently does not support themes)" << "\n";
			createDarkTheme << "//scrollbar_scrollbar = 80, 80, 80" << "\n";
			createDarkTheme << "//scrollbar_scrollthumb = 110, 110, 110" << "\n";
			createDarkTheme << "//scrollbar_scrollthumb_hover = 158, 158, 158" << "\n";
			createDarkTheme << "//scrollbar_scrollthumb_hold = 239, 235, 339" << "\n";
			createDarkTheme << "\n";
			createDarkTheme << "// items" << "\n";
			createDarkTheme << "item_card = 100, 100, 100" << "\n";
			createDarkTheme << "item_icon = 255, 255, 255" << "\n";
			createDarkTheme << "item_icon_hover = 255, 255, 255" << "\n";
			createDarkTheme << "item_redownload = 255, 255, 255" << "\n";
			createDarkTheme << "item_update_is_available = 255, 200, 0" << "\n";
			createDarkTheme << "item_download = 255, 255, 255" << "\n";
			createDarkTheme << "\n";
			createDarkTheme << "// links" << "\n";
			createDarkTheme << "link_card = 100, 100, 100" << "\n";
			createDarkTheme << "link_follow = 255, 200, 0" << "\n";
			createDarkTheme << "link_text = 0, 170, 232" << "\n";

			createDarkTheme.close();
		}

		if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "light.sfuitheme"))
		{
			progressBar->addThingToDo();
			std::cout << "creating theme file" << "\n";
			std::ofstream createLightTheme(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + "light.sfuitheme");

			createLightTheme << "// default 'light' theme for kunlauncher" << "\n";
			createLightTheme << "\n";
			createLightTheme << "// globals" << "\n";
			createLightTheme << "global_background = 255, 255, 255" << "\n";
			createLightTheme << "global_text = 0, 0, 0" << "\n";
			createLightTheme << "\n";
			createLightTheme << "// scrollbar (useless because SFUI currently does not support themes)" << "\n";
			createLightTheme << "//scrollbar_scrollbar = 80, 80, 80" << "\n";
			createLightTheme << "//scrollbar_scrollthumb = 110, 110, 110" << "\n";
			createLightTheme << "//scrollbar_scrollthumb_hover = 158, 158, 158" << "\n";
			createLightTheme << "//scrollbar_scrollthumb_hold = 239, 235, 339" << "\n";
			createLightTheme << "\n";
			createLightTheme << "// items" << "\n";
			createLightTheme << "item_card = 192, 192, 192" << "\n";
			createLightTheme << "item_icon = 255, 255, 255" << "\n";
			createLightTheme << "item_icon_hover = 255, 255, 255" << "\n";
			createLightTheme << "item_redownload = 255, 255, 255" << "\n";
			createLightTheme << "item_update_is_available = 255, 200, 0" << "\n";
			createLightTheme << "item_download = 255, 255, 255" << "\n";
			createLightTheme << "\n";
			createLightTheme << "// links" << "\n";
			createLightTheme << "link_card = 100, 100, 100" << "\n";
			createLightTheme << "link_follow = 255, 200, 0" << "\n";
			createLightTheme << "link_text = 0, 170, 232" << "\n";

			createLightTheme.close();
		}

		std::cout << "loading theme settings" << "\n";

		SettingsParser settings;
		if (settings.loadFromFile(".\\" + GBL::DIR::BASE + "kunlauncher.conf"))
			settings.get("defaultTheme", app->settings.theme);
		std::cout << app->settings.theme << "\n";

		if (settings.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + app->settings.theme + ".sfuitheme"))
		{
			std::cout << "loaded theme \"" << app->settings.theme << "\"." << "\n";

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
			std::cout << "scrollbar theme settings" << "\n";

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
			std::cout << "item theme settings" << "\n";

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

			std::cout << "done applying themes" << "\n";
		}
		else
		{
			std::cout << "failed to load settings file" << "\n"; // use default colours
		}

		progressBar->oneThingDone();
	}
	progressBar->oneThingDone(); // 8

	return 0;
}

int InitialiseState::validateResourceFiles()
{
	setTaskText("validating resource files");

	progressBar->addThingToDo();

	if (fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "textures.dat"))
	{
		SettingsParser getTextures;
		std::vector<std::string> textures;
		getTextures.get("textures", textures);
		progressBar->addThingsToDo(textures.size());

		for (size_t i = 0; i < textures.size(); i++)
		{
			if (fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::TEXTURE + textures[i]))
			{
				std::cout << textures[i] << " exsits." << "\n";
			}

			progressBar->oneThingDone();
		}
	}
	else
	{
		std::cout << (".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "textures.dat") << " does not exist" << "\n"; 
	}

	progressBar->oneThingDone();

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

// move theme stuff out of validateFIleStructure
// that function should only check for files and create them if they do not exist
