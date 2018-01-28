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

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

InitialiseState InitialiseState::IntialiseState_dontfuckwithme;

namespace fs = std::experimental::filesystem;

void InitialiseState::Init(AppEngine* app_)
{
	initTime.restart();
	std::cout << "IntialiseState Init" << std::endl;

	app = app_;
	
	if (!font.loadFromFile(GBL::DIR::fonts + "//Arial.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << std::endl;

		if (!font.loadFromFile("C://Windows//Fonts//Arial.ttf"))
		{
			std::cout << "failed to load a font!" << std::endl;

			abort();
		}
	}

	app->window->create(sf::VideoMode(400, 150), app->title, sf::Style::None);
	app->window->setVerticalSyncEnabled(app->settings.verticalSync);
	app->window->setTitle("KunLauncher " + std::to_string(GBL::VERSION) + " initalising");

	initialiseText.setFont(font);
	initialiseText.setCharacterSize(56);
	initialiseText.setString("KunLauncher");
	initialiseText.setOrigin(initialiseText.getLocalBounds().width / 2, initialiseText.getLocalBounds().height - 20);
	initialiseText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y / 2)));

	thingthatdoesntripoffvisualstudio.setSize(sf::Vector2f(app->window->getSize().x, 20));
	thingthatdoesntripoffvisualstudio.setFillColor(sf::Color(100, 100, 100));
	thingthatdoesntripoffvisualstudio.setPosition(sf::Vector2f(0, (app->window->getSize().y - 20)) );

	progressBar = new ProgressBar(sf::Vector2f(0, (app->window->getSize().y - 20)), thingthatdoesntripoffvisualstudio.getSize().x, thingthatdoesntripoffvisualstudio.getSize().y);

	currentLauncherTask.setFont(font);
	currentLauncherTask.setCharacterSize(20);

	setTaskText("You can't see me!");

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

	sf::RenderWindow* newWindow = new sf::RenderWindow(sf::VideoMode(app->settings.width, app->settings.height), "KunLauncher " + std::to_string(GBL::VERSION), sf::Style::Resize | sf::Style::Close);
	newWindow->setVerticalSyncEnabled(true);
	newWindow->setKeyRepeatEnabled(false);

	app->window->close();
	delete app->window;
	app->window = newWindow;

	std::cout << "Initalisation took " << initTime.getElapsedTime().asSeconds() << "s" << std::endl;
	std::cout << "IntialiseState Cleanedup" << std::endl;
}

void InitialiseState::Pause()
{
	std::cout << "IntialiseState Pause" << std::endl;
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

			//TODO: cross platform support that is not shit :D
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
	{ // always do this first.
		progressBar->reset();
		progressBar->addThingToDo();
		setTaskText("loading configuration");
		SettingsParser settings;
		if (settings.loadFromFile(GBL::DIR::config))
		{
			settings.get("min_window_width", app->settings.width);
			settings.get("min_window_height", app->settings.height);
			settings.get("update_launcher_on_start", app->settings.updateLauncherOnStart);
			settings.get("check_for_items_on_start", app->settings.checkForNewItemsOnStart);
			settings.get("print_download_progress", app->settings.printdownloadprogress);
			settings.get("selected_theme", app->settings.selectedTheme);
		}
		else
		{
			std::cout << "failed to load settings, using defaults" << std::endl;
		}
		progressBar->oneThingDone();
	}

	validateFileStructure();
	validateResourceFiles();
	getThemeConfiguration();

	if (app->settings.checkForNewItemsOnStart)
	{
		progressBar->reset();
		progressBar->addThingsToDo(2);
		setTaskText("updating app index");
		std::cout << "updating app index" << std::endl;

		Download getIndex;
		getIndex.setInput(GBL::WEB::APPS + "/index.dat");
		getIndex.setOutputDir(GBL::DIR::apps);
		getIndex.setOutputFilename("//index.dat");

		getIndex.download();
		progressBar->oneThingDone();
		getIndex.save();
		progressBar->oneThingDone();
	}
	else
	{
		std::cout << "skipping item update" << std::endl;
	}

	if (app->settings.updateLauncherOnStart)
	{
		progressBar->reset();
		progressBar->addThingsToDo(2);
		setTaskText("checking for updates");

		if (fs::exists("kunlauncher.exe.old"))
		{
			try
			{
				fs::remove("kunlauncher.exe.old");
			}
			catch (const std::exception& e)
			{
				std::cerr << "failed to remove kunlauncher.exe.old" << std::endl;
				std::cerr << e.what() << std::endl;
			}

			progressBar->oneThingDone();
		}

		LauncherUpdater *updater = new LauncherUpdater;
		//TODO: this probably isn't the best way to see if updates are available
		int updateStatus = updater->checkForUpdates();

		progressBar->oneThingDone(); // check for update

		if (updateStatus == LauncherUpdater::Status::RequiredUpdate)
		{
			std::cout << "forcing client update" << std::endl;

			progressBar->reset();
			progressBar->addThingsToDo(2); // update and replace exe

			setTaskText("downloading update");
			updater->downloadUpdate();
			progressBar->oneThingDone(); // update

			setTaskText("replacing old executable");
			updater->replaceOldExecutable();
			progressBar->oneThingDone(); // replace exe

			restartNow = true;
		}
		else if (updateStatus == LauncherUpdater::Status::UpdateAvailable)
		{
			MessageBox::Options modOptions;
			//TODO: add error handling
			modOptions.title = "Update Available";
			modOptions.text = "Version " + std::to_string(updater->remoteVersion) + " is available, would you like to update?";
			modOptions.settings = { "Yes", "No" };

			MessageBox doYouWannaUpdate(modOptions);
			doYouWannaUpdate.runBlocking();
			doYouWannaUpdate.close(); // right after it's done

			switch (doYouWannaUpdate.returnCode)
			{
			case 0:
			{
				progressBar->reset();
				progressBar->addThingsToDo(2); // update and replace exe
				setTaskText("updating launcher");

				setTaskText("downloading update");
				updater->downloadUpdate();
				progressBar->oneThingDone(); // update

				setTaskText("replacing old executable");
				updater->replaceOldExecutable();
				progressBar->oneThingDone(); // replace exe

				MessageBox::Options modOptions;
				modOptions.text = "Launcher updated";

				modOptions.text = "Launcher updated to v" + std::to_string(updater->remoteVersion) + "! Restart?";
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
		progressBar->oneThingDone(); // check for update
	}

	setTaskText("ready");

	helperDone = true;
}

int InitialiseState::validateFileStructure()
{
	progressBar->reset();
	progressBar->addThingsToDo(7); // bin, config, apps, app index, resources, stats
	setTaskText("validating files");

	std::cout << "checking for bin" << std::endl; { // 1
		if (!fs::exists(GBL::DIR::installDir))
		{
			std::cout << "bin folder missing, creating" << std::endl;
			fs::create_directories(GBL::DIR::installDir);
		}
	} progressBar->oneThingDone();

	std::cout << "checking for cache" << std::endl; { // 2
		if (!fs::exists(GBL::DIR::cache))
		{
			std::cout << "creating cache folder" << std::endl;
			fs::create_directory(GBL::DIR::cache);
		}
	} progressBar->oneThingDone();

	std::cout << "checking for config" << std::endl; { // 3
		if (!fs::exists(GBL::DIR::config))
		{
			std::cout << "config file missing, creating" << std::endl;

			std::ofstream createConfigurationFile(GBL::DIR::config);

			createConfigurationFile << "// default configuration for kunlauncher" << std::endl;
			createConfigurationFile << std::endl;
			createConfigurationFile << "min_window_width = 525" << std::endl;
			createConfigurationFile << "min_window_height = 400" << std::endl;
			createConfigurationFile << "update_launcher_on_start = TRUE" << std::endl;
			createConfigurationFile << "check_for_items_on_start = TRUE" << std::endl;
			createConfigurationFile << "print_download_progress = TRUE" << std::endl;
			createConfigurationFile << "selected_theme = dark" << std::endl;
			createConfigurationFile << std::endl;
			createConfigurationFile << "// default controls:" << std::endl;
			createConfigurationFile << "// Right Click: go back" << std::endl;
			createConfigurationFile << "// R: Reload Apps list" << std::endl;
			createConfigurationFile << "// Control + R: Redownload app manifest and reload app list" << std::endl;
			createConfigurationFile << "// Control + Shift + R: delete all apps, redownload app manifest, and reload applist" << std::endl;

			createConfigurationFile.close();
		}
	} progressBar->oneThingDone();
	
	std::cout << "checking for third party notices" << std::endl; { // 4
		if (!fs::exists(GBL::DIR::installDir + "thirdpartynotices.txt")) // 4
		{
			setTaskText("retrieving thirdpartynotices");
			std::cout << "missing thirdpartynotices, downloading..." << std::endl;

			Download getThirdPartyNotices;
			getThirdPartyNotices.setInput(GBL::WEB::LATEST::DIR + "/thirdpartynotices.txt");
			getThirdPartyNotices.setOutputDir(GBL::DIR::installDir);
			getThirdPartyNotices.setOutputFilename("//thirdpartynotices.txt");
			getThirdPartyNotices.download();
			getThirdPartyNotices.save();

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

			setTaskText("validating files");
		}
	} progressBar->oneThingDone(); 

	std::cout << "checking for apps" << std::endl; { // 5
		if (!fs::exists(GBL::DIR::apps))
		{
			std::cout << "apps folder missing, creating" << std::endl;
			fs::create_directory(GBL::DIR::apps);
		}
	} progressBar->oneThingDone();

	std::cout << "checking for apps+index" << std::endl; {
		if (!fs::exists(GBL::DIR::apps + "index.dat")) // 6
		{
			std::cout << "app index missing, creating" << std::endl;

			Download getItemIndex;
			getItemIndex.setInput(GBL::WEB::APPS + "/index.dat");
			getItemIndex.setOutputDir(GBL::DIR::apps);
			getItemIndex.setOutputFilename("//index.dat");
			getItemIndex.download();
			getItemIndex.save();

			setTaskText("validating files");
		}
	} progressBar->oneThingDone();

	std::cout << "checking for resource folder" << std::endl; { // 6
		if (!fs::exists(GBL::DIR::resources))
		{
			std::cout << "resource folder is missing, creating" << std::endl;

			try
			{
				fs::create_directory(GBL::DIR::resources);
			}
			catch (const std::exception& e)
			{
				std::cerr << "failed to create resources folder:" << std::endl;
				std::cerr << e.what() << std::endl;
			}
		}
	} progressBar->oneThingDone();

	std::cout << "checking for themes folder" << std::endl; { // 8
		if (!fs::exists(GBL::DIR::themes))
		{
			std::cout << "themes folder is missing, creating" << std::endl;

			try
			{
				fs::create_directory(GBL::DIR::themes);
			}
			catch (const std::exception& e)
			{
				std::cerr << "failed to create themes folder:" << std::endl;
				std::cerr << e.what() << std::endl;
			}
		}
	} progressBar->oneThingDone();

	std::cout << "checking for stats folder" << std::endl; { // 8
		if (!fs::exists(GBL::DIR::stats))
		{
			std::cout << "stats folder is missing, creating" << std::endl;

			try
			{
				fs::create_directory(GBL::DIR::stats);

				std::ofstream createStatisticsFile(GBL::DIR::stats + "appstats.dat");

				if (createStatisticsFile.is_open())
				{
					createStatisticsFile << "averageAppCreationTime = 0" << std::endl;
				}
				else
				{
					std::cerr << "failed to app stats file" << std::endl;
				}
			}
			catch (const std::exception& e)
			{
				std::cerr << "failed to create stats folder:" << std::endl;
				std::cerr << e.what() << std::endl;
			}
		}
	} progressBar->oneThingDone();

	return 0;
}

int InitialiseState::validateResourceFiles()
{
	setTaskText("validating resource files");
	progressBar->reset();
	progressBar->addThingsToDo(2); // 1 make sure we have resources.dat, 2 make sure it's up to date

	if (!fs::exists(GBL::DIR::resources + "resources.dat"))
	{
		std::ofstream createResourcesManifest(GBL::DIR::resources + "resources.dat");
		createResourcesManifest.close();
	}
	progressBar->oneThingDone();

	Download getResourceManifest;
	getResourceManifest.setInput(GBL::WEB::LATEST::RESOURCES + "resources.dat");

	if (getResourceManifest.getFileSize() != fs::file_size(GBL::DIR::resources + "resources.dat"))
	{
		std::cout << "resources have been updated" << std::endl;

		getResourceManifest.setOutputDir(GBL::DIR::resources);
		getResourceManifest.setOutputFilename("//resources.dat");
		getResourceManifest.download();
		getResourceManifest.save();
	}
	else
	{
		std::cout << "resource manifest up to date" << std::endl;

		std::cout << getResourceManifest.getFileSize() << std::endl;
		std::cout << fs::file_size(GBL::DIR::resources + "resources.dat") << std::endl;
	}
	progressBar->oneThingDone();

	SettingsParser getResources;

	if (getResources.loadFromFile(GBL::DIR::resources + "resources.dat"))
	{
		std::vector<std::string> textures;
		std::vector<std::string> fonts;

		getResources.get("textures", textures);
		getResources.get("fonts", fonts);

		progressBar->addThingsToDo(textures.size() + fonts.size());

		for (size_t i = 0; i < textures.size(); i++)
		{
			if (fs::exists(GBL::DIR::textures + textures[i]))
			{
				std::cout << textures[i] << " exists, next." << std::endl;
			}
			else
			{
				std::cout << std::endl;
				setTaskText("retrieving " + textures[i]);
				std::cout << textures[i] << " is missing, downloading." << std::endl;

				Download getTexture;
				getTexture.setInput(".//" + GBL::WEB::LATEST::RESOURCES + "/textures/" + textures[i]);
				getTexture.setOutputDir(GBL::DIR::textures);
				getTexture.setOutputFilename("//" + textures[i]);
				getTexture.download();
				getTexture.save();
			}

			progressBar->oneThingDone();
		}

		for (size_t i = 0; i < fonts.size(); i++)
		{
			if (fs::exists(GBL::DIR::fonts + fonts[i]))
			{
				std::cout << fonts[i] << " exists, next." << std::endl;
			}
			else
			{
				std::cout << std::endl;
				setTaskText("retrieving " + fonts[i]);
				std::cout << fonts[i] << " is missing, downloading." << std::endl;

				Download getFont;
				getFont.setInput(".//" + GBL::WEB::LATEST::RESOURCES + "/fonts/" + fonts[i]);
				getFont.setOutputDir(GBL::DIR::fonts);
				getFont.setOutputFilename("//" + fonts[i]);
				getFont.download();
				getFont.save();
			}

			progressBar->oneThingDone();
		}

		return 1;
	}
	else
	{
		std::cerr << "failed to open resource manifest for verification" << std::endl;
		return 0;
	}
}

int InitialiseState::getThemeConfiguration()
{
	std::cout << "checking for theme file" << std::endl;

	if (!fs::exists(GBL::DIR::themes + "dark.sfuitheme"))
	{
		std::cout << "creating theme file" << std::endl;
		std::ofstream createDarkTheme(GBL::DIR::themes + "dark.sfuitheme");

		createDarkTheme << "// default 'dark' theme for kunlauncher" << std::endl;
		createDarkTheme << std::endl;
		createDarkTheme << "// globals" << std::endl;
		createDarkTheme << "global_background = 50, 50, 50" << std::endl;
		createDarkTheme << "global_text = 255, 255, 255" << std::endl;
		createDarkTheme << std::endl;
		createDarkTheme << "// scrollbar" << std::endl;
		createDarkTheme << "scrollbar_scrollbar = 80, 80, 80" << std::endl;
		createDarkTheme << "scrollbar_scrollthumb = 110, 110, 110" << std::endl;
		createDarkTheme << "scrollbar_scrollthumb_hover = 158, 158, 158" << std::endl;
		createDarkTheme << "scrollbar_scrollthumb_hold = 239, 235, 239" << std::endl;
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

	if (!fs::exists(GBL::DIR::themes + "light.sfuitheme"))
	{
		std::cout << "creating theme file" << std::endl;
		std::ofstream createLightTheme(GBL::DIR::themes + "light.sfuitheme");

		createLightTheme << "// default 'light' theme for kunlauncher" << std::endl;
		createLightTheme << std::endl;
		createLightTheme << "// globals" << std::endl;
		createLightTheme << "global_background = 255, 255, 255" << std::endl;
		createLightTheme << "global_text = 0, 0, 0" << std::endl;
		createLightTheme << std::endl;
		createLightTheme << "// scrollbar" << std::endl;
		createLightTheme << "scrollbar_scrollbar = 80, 80, 80" << std::endl;
		createLightTheme << "scrollbar_scrollthumb = 110, 110, 110" << std::endl;
		createLightTheme << "scrollbar_scrollthumb_hover = 158, 158, 158" << std::endl;
		createLightTheme << "scrollbar_scrollthumb_hold = 239, 235, 239" << std::endl;
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
	if (settings.loadFromFile(GBL::DIR::config))
		settings.get("selected_theme", app->settings.selectedTheme);
	std::cout << app->settings.selectedTheme << std::endl;

	if (settings.loadFromFile(GBL::DIR::themes + app->settings.selectedTheme + ".sfuitheme"))
	{
		std::cout << "loaded theme \"" << app->settings.selectedTheme << "\"." << std::endl;

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

		//scrollbar
		std::cout << "scrollbar theme settings" << std::endl;

		if (settings.get("scrollbar_scrollbar", colors))
		{
			GBL::COLOR::SCROLLBAR::SCROLLBAR = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		if (settings.get("scrollbar_scrollthumb", colors))
		{
			GBL::COLOR::SCROLLBAR::SCROLLTHUMB = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		if (settings.get("scrollbar_scrollthumb_hover", colors))
		{
			GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		if (settings.get("scrollbar_scrollthumb_hold", colors))
		{
			GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOLD = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

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
		std::cerr << "failed to load settings file" << std::endl; // use default colours
	}

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

// move theme stuff out of validateFIleStructure
// that function should only check for files and create them if they do not exist
