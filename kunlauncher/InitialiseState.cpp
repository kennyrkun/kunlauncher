#include "AppEngine.hpp"
#include "InitialiseState.hpp"
#include "HomeState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "LauncherUpdater.hpp"
#include "ProgressBar.hpp"
#include "SettingsParser.hpp"

#include <SFML/Graphics.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

// TODO: refactor init system
// TODO: proper Theme system.
// each them can have it's own assets in it's own folder in themes/

namespace fs = std::experimental::filesystem;

void InitialiseState::Init(AppEngine* app_)
{
	initTime.restart();
	std::cout << "IntialiseState Init" << std::endl;

	app = app_;

	// HACK: this is nasty
//	font = GBL::theme.getFont("Arial.ttf");
//	if (font == nullptr)
	{
		std::cerr << "failed to load provided arial, falling back to windows Arial!" << std::endl;

		if (!font.loadFromFile("C://Windows//Fonts//Arial.ttf"))
		{
			std::cerr << "failed to load a font!" << std::endl;
		}
	}

	app->window->create(sf::VideoMode(400, 150), GBL::appName, sf::Style::None);
	app->window->setVerticalSyncEnabled(app->settings.window.verticalSync);
	app->window->setTitle("KunLauncher initalising");

	initialiseText.setFont(font);
	initialiseText.setCharacterSize(56);
	initialiseText.setString("KunLauncher");
	initialiseText.setOrigin(sf::Vector2f(initialiseText.getLocalBounds().width / 2.0f, initialiseText.getLocalBounds().height - 20.0f));
	initialiseText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y / 2.0f)));

	thingthatdoesntripoffvisualstudio.setSize(sf::Vector2f(app->window->getSize().x, 20.0f));
	thingthatdoesntripoffvisualstudio.setFillColor(sf::Color(100, 100, 100));
	thingthatdoesntripoffvisualstudio.setPosition(sf::Vector2f(0.0f, (app->window->getSize().y - 20.0f)) );

	progressBar = new ProgressBar(sf::Vector2f(0.0f, (app->window->getSize().y - 20.0f)), thingthatdoesntripoffvisualstudio.getSize().x, thingthatdoesntripoffvisualstudio.getSize().y);

	currentLauncherTask.setFont(font);
	currentLauncherTask.setCharacterSize(20);
	setTaskText("You can't see me!");

	// FIXME: this prints an error to the console.
	// the program continues fine without it,
	// and in fact would crash without it.
	// though it does produce an error, which is bad.
	// fixy fixy.
	app->window->clear(sf::Color(50, 50, 50));
	app->window->draw(initialiseText);
	app->window->draw(currentLauncherTask);
	app->window->draw(thingthatdoesntripoffvisualstudio);
	app->window->draw(*progressBar);
	app->window->display();

	app->window->setActive(false); // we don't need it in this thread anymore

	while (!app->window->isOpen())
		std::cout << "waiting" << std::endl;

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
	sf::RenderWindow* newWindow = new sf::RenderWindow(sf::VideoMode(app->settings.window.width, app->settings.window.height),
														"KunLauncher", sf::Style::Resize | sf::Style::Close);

	newWindow->setVerticalSyncEnabled(app->settings.window.verticalSync);

	sf::Image icon;
	if (icon.loadFromFile(GBL::DIR::textures + "get_app_1x.png"))
		newWindow->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	else
		derr("failed to load application icon");

	app->window->close();
	delete app->window;

	app->window = newWindow;
	app->window->requestFocus();

	app->navbar = new Navbar(app->window, app->am);
	app->navbar->addSection("home");
	app->navbar->addSection("my apps");
	app->navbar->addSection("all apps");
	app->navbar->addSection("settings");

	std::cout << "Initalisation took " << initTime.getElapsedTime().asSeconds() << "s" << std::endl;
	std::cout << "Cleaned up IntialiseState" << std::endl;
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

	// TODO: allow movement of window

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
			return;
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::LShift)
			{
				progressBar->setColor(sf::Color::Red, sf::Color::Green, sf::Color::Magenta, sf::Color::Blue);

				std::cout << "developer mode toggled" << std::endl;
			}
		}
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
		{
			app->ChangeState(new HomeState);
			return;
		}
		

		return;
	}
}

void InitialiseState::Draw()
{

}

void InitialiseState::initialise()
{
	// FIXME: fixy fixy
	if (!app->window->setActive(true))
		abort();
//		std::cerr << "failed to get window context" << std::endl;

	{ // always do this first.
		progressBar->reset();
		progressBar->addThingToDo();

		setTaskText("loading configuration");

		SettingsParser settings;
		if (settings.loadFromFile(GBL::CONFIG::config))
		{
			settings.get(GBL::CONFIG::updateOnStart, app->settings.updateOnStart);
			settings.get(GBL::CONFIG::logDownloads, app->settings.logDownloads);
			settings.get(GBL::CONFIG::SFUIDebug, app->settings.SFUIDebug);
			settings.get(GBL::CONFIG::allowStatTracking, app->settings.allowStatTracking);
			settings.get(GBL::CONFIG::useAnimations, app->settings.useAnimations);
			settings.get(GBL::CONFIG::animationScale, app->settings.animationScale);

			settings.get(GBL::CONFIG::News::newsEnabled, app->settings.news.enabled);

			settings.get(GBL::CONFIG::selectedTheme, app->settings.selectedTheme);

			settings.get(GBL::CONFIG::Window::verticalSync, app->settings.window.verticalSync);
			settings.get(GBL::CONFIG::Window::width, app->settings.window.width);
			settings.get(GBL::CONFIG::Window::height, app->settings.window.height);

			settings.get(GBL::CONFIG::Apps::updateStoreOnStart, app->settings.apps.updateStoreOnStart);
			settings.get(GBL::CONFIG::Apps::autoUpdate, app->settings.apps.autoUpdate);
			settings.get(GBL::CONFIG::Apps::checkForUpdates, app->settings.apps.checkForUpdates);

			settings.print();
		}
		else
		{
			std::cerr << "failed to load settings, using defaults" << std::endl;
		}

		progressBar->oneThingDone();
	}

	// make sure we have all the folders and such
	validateFileStructure();
	// make sure we have all the textures and such
	validateResourceFiles();
	// make sure we have at least the default themes
	getThemeConfiguration();

	if (GBL::theme.loadFromFile(app->settings.selectedTheme))
		app->SetMultiThreadedIndicatorIcon(GBL::theme.getTexture("settings_2x.png"));

	SFUI::Theme::loadFont(GBL::theme.getFont("Arial.ttf"));
	SFUI::Theme::loadTexture(GBL::theme.getTexture("interface_square.png"));

	if (app->settings.apps.updateStoreOnStart)
	{
		progressBar->reset();
		progressBar->addThingsToDo(2);
		setTaskText("updating app index");
		std::cout << "updating app index" << std::endl;

		Download getIndex;
		getIndex.setInput(GBL::WEB::APPS + "index.dat");
		getIndex.setOutputDir(GBL::DIR::appcache);
		getIndex.setOutputFilename("/index.dat");

		if (getIndex.download())
		{
			progressBar->oneThingDone();

			if (getIndex.save())
				progressBar->oneThingDone();
		}
		else
		{
			std::cerr << "failed to download index" << std::endl;
		}
	}
	else
	{
		std::cout << "skipping item update" << std::endl;
	}

	if (app->settings.updateOnStart)
	{
		progressBar->reset();
		progressBar->addThingsToDo(3);

		setTaskText("checking for old executable");
		if (fs::exists("kunlauncher.exe.old"))
		{
			try
			{
				setTaskText("removing old executable");
				fs::remove("kunlauncher.exe.old");
			}
			catch (const std::exception& e)
			{
				std::cerr << "failed to remove kunlauncher.exe.old" << std::endl;
				std::cerr << e.what() << std::endl;
			}

			progressBar->oneThingDone();
		}
		progressBar->oneThingDone();

		setTaskText("checking for updates");
		LauncherUpdater updater;
		int updateStatus = updater.checkForUpdates();
		progressBar->oneThingDone(); // check for update

		if (updateStatus == LauncherUpdater::Status::RequiredUpdate)
		{
			std::cout << "update is required" << std::endl;

			progressBar->reset();
			progressBar->addThingsToDo(2); // update and replace exe

			setTaskText("downloading update");

			if (!updater.downloadUpdate())
			{
				std::cerr << "UPDATE FAILED" << std::endl;
				abort();
			}
			progressBar->oneThingDone(); // update

			setTaskText("finishing update");
			updater.replaceOldExecutable();
			progressBar->oneThingDone(); // replace exe

			restartNow = true;
		}
		else if (updateStatus == LauncherUpdater::Status::UpdateAvailable)
		{
			MessageBox::Options modOptions;
			//TODO: add error handling
			modOptions.title = "Update Available";
			modOptions.text = "release " + std::to_string(updater.remoteRelease) + " is available, would you like to update?";
			modOptions.settings = { "Yes", "No" };

			MessageBox doYouWannaUpdate(modOptions);
			doYouWannaUpdate.runBlocking();
			doYouWannaUpdate.close(); // right after it's done

			switch (doYouWannaUpdate.exitCode)
			{
			case 0: // yes
			{
				progressBar->reset();
				progressBar->addThingsToDo(2); // update and replace exe

				setTaskText("downloading update");
				updater.downloadUpdate();
				progressBar->oneThingDone(); // update

				setTaskText("finishing update");
				updater.replaceOldExecutable();
				progressBar->oneThingDone(); // replace exe

				MessageBox::Options updateSuccessful_options; // restart options
				modOptions.text = "Update Complete";

				// TODO: don't allow restart now on platforms other than windows (we don't know how to start the applicationa again)
				updateSuccessful_options.text = "The launcher has been updated to release " + std::to_string(updater.remoteRelease) + "\n A restart is required.";
				updateSuccessful_options.settings = { "Restart Now", "Restart Later" };

				MessageBox updateSuccessfulModal(updateSuccessful_options);
				updateSuccessfulModal.runBlocking();

				switch (updateSuccessfulModal.exitCode)
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
			case 1: // no
			{
				std::cout << "don't update now" << std::endl;
				break;
			}
			}
		}
	}
	else
	{
		std::cout << "skipping check for updates" << std::endl;
	}

	setTaskText("ready");

	app->window->setActive(false);

	helperDone = true;
}

int InitialiseState::validateFileStructure()
{
	progressBar->reset();
	progressBar->addThingsToDo(10); // bin, cache, appcache, config, tpn, apps, apps+index, resources, themes, stats
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

	std::cout << "checking for appcache" << std::endl; { // 3
		if (!fs::exists(GBL::DIR::appcache))
		{
			std::cout << "creating appcache folder" << std::endl;
			fs::create_directory(GBL::DIR::appcache);
		}
	} progressBar->oneThingDone();

	std::cout << "checking for config" << std::endl; { // 4
		if (!fs::exists(GBL::CONFIG::config))
		{
			std::cout << "config file missing, creating" << std::endl;

			std::ofstream createConfigurationFile(GBL::CONFIG::config);

			if (createConfigurationFile.is_open())
			{
				createConfigurationFile << "// default configuration for kunlauncher" << std::endl;
				createConfigurationFile << std::endl;

				createConfigurationFile << GBL::CONFIG::updateOnStart << " = " << (app->settings.updateOnStart ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::logDownloads << " = " << (app->settings.logDownloads ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::SFUIDebug << " = " << (app->settings.SFUIDebug ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::allowStatTracking << " = " << (app->settings.allowStatTracking ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::useAnimations << " = " << (app->settings.useAnimations ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::animationScale << " = " << app->settings.animationScale << std::endl;

				createConfigurationFile << GBL::CONFIG::News::newsEnabled << " = " << (app->settings.news.enabled ? "TRUE" : "FALSE") << std::endl;

				createConfigurationFile << GBL::CONFIG::selectedTheme<< " = " << app->settings.selectedTheme << std::endl;

				createConfigurationFile << GBL::CONFIG::Window::verticalSync << " = " << (app->settings.window.verticalSync ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::Window::width << " = " << app->settings.window.width << std::endl;
				createConfigurationFile << GBL::CONFIG::Window::height << " = " << app->settings.window.height << std::endl;

				createConfigurationFile << GBL::CONFIG::Apps::updateStoreOnStart << " = " << (app->settings.apps.updateStoreOnStart ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::Apps::autoUpdate << " = " << (app->settings.apps.autoUpdate ? "TRUE" : "FALSE") << std::endl;
				createConfigurationFile << GBL::CONFIG::Apps::checkForUpdates << " = " << (app->settings.apps.checkForUpdates ? "TRUE" : "FALSE") << std::endl;

				createConfigurationFile << std::endl;

				createConfigurationFile << "// Keys:" << std::endl;
				createConfigurationFile << "// R: Reload Apps list" << std::endl;
				createConfigurationFile << "// Control + R: Redownload app index and reload app list" << std::endl;
				createConfigurationFile << "// Control + Shift + R: delete all apps, redownload app index, and reload applist" << std::endl;

				createConfigurationFile.close();

				if (createConfigurationFile.bad())
				{
					std::cerr << "failed to save default config file (it may be corrupt)" << std::endl;
				}
			}
			else
			{
				std::cerr << "failed to create default configuration file" << std::endl;
			}
		}
	} progressBar->oneThingDone();
	
	std::cout << "checking for third party notices" << std::endl; { // 5
		if (!fs::exists(GBL::DIR::installDir + "thirdpartynotices.txt"))
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
				std::cout << "failed to upload third party notices." << std::endl;
			}
			default:
				break;
			}

			setTaskText("validating files");
		}
	} progressBar->oneThingDone(); 

	std::cout << "checking for apps" << std::endl; { // 6
		if (!fs::exists(GBL::DIR::apps))
		{
			std::cout << "apps folder missing, creating" << std::endl;
			fs::create_directory(GBL::DIR::apps);
		}
	} progressBar->oneThingDone();

	std::cout << "checking for appcache+index" << std::endl; {
		if (!fs::exists(GBL::DIR::appcache + "index.dat")) // 7
		{
			std::cout << "appcache index missing, creating" << std::endl;

			Download getItemIndex;
			getItemIndex.setInput(GBL::WEB::APPS + "/index.dat");
			getItemIndex.setOutputDir(GBL::DIR::appcache);
			getItemIndex.setOutputFilename("/index.dat");
			getItemIndex.download();
			getItemIndex.save();
		}
	} progressBar->oneThingDone();

	std::cout << "checking for resource folder" << std::endl; { // 8
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

	std::cout << "checking for themes folder" << std::endl; { // 9
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

	std::cout << "checking for stats folder" << std::endl; { // 10
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
		getResourceManifest.setOutputDir(GBL::DIR::resources);
		getResourceManifest.setOutputFilename("//resources.dat");
		getResourceManifest.download();
		getResourceManifest.save();

		std::cout << "resources have been updated" << std::endl;
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
			if (!fs::exists(GBL::DIR::textures + textures[i]))
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
			if (!fs::exists(GBL::DIR::fonts + fonts[i]))
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
	}
	else
	{
		std::cerr << "failed to open resource manifest for verification" << std::endl;
		return 0;
	}

	return 1;
}

int InitialiseState::getThemeConfiguration()
{
	// TODO: move theme stuff out of validateFIleStructure
	// that function should only check for files and create them if they do not exist

	std::cout << "checking for theme files" << std::endl;

	setTaskText("loading theme");

	if (!fs::exists(GBL::DIR::themes + "dark/dark.sfuitheme"))
	{
		if (!fs::exists(GBL::DIR::themes + "dark"))
		{
			try
			{
				fs::create_directory(GBL::DIR::themes + "dark");

				std::cout << "creating dark theme file" << std::endl;
				std::ofstream darkTheme(GBL::DIR::themes + "dark/dark.sfuitheme");

				darkTheme << "// default 'dark' theme for kunlauncher" << std::endl;
				darkTheme << std::endl;
				darkTheme << "name = dark" << std::endl;
				darkTheme << "author = kennyrkun" << std::endl;
				darkTheme << "release = 1" << std::endl;
				darkTheme << "launcherrelease = 17" << std::endl;
				darkTheme << std::endl;
				darkTheme << "// globals" << std::endl;
				darkTheme << "primary = 100, 100, 100" << std::endl;
				darkTheme << "secondary = 150, 150, 150" << std::endl;
				darkTheme << "tertiary = 50, 50, 50" << std::endl;
				darkTheme << "text = 255, 255, 255" << std::endl;
				darkTheme << "text_secondary = 255, 255, 255" << std::endl;
				darkTheme << std::endl;
				darkTheme << "// scrollbar" << std::endl;
				darkTheme << "scrollbar = 80, 80, 80" << std::endl;
				darkTheme << "scrollthumb = 110, 110, 110" << std::endl;
				darkTheme << "scrollthumb_hover = 158, 158, 158" << std::endl;
				darkTheme << "scrollthumb_hold = 239, 235, 239" << std::endl;
				darkTheme << std::endl;
				darkTheme << "// apps" << std::endl;
				darkTheme << "app_card = 100, 100, 100" << std::endl;
				darkTheme << "app_card2 = 100, 100, 100" << std::endl;
				darkTheme << "app_image = 255, 255, 255" << std::endl;
				darkTheme << "app_icon = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_hover = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_press = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_fail = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_fail_hover = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_fail_press = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_warn = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_warn_hover = 255, 255, 255" << std::endl;
				darkTheme << "app_icon_warn_press = 255, 255, 255" << std::endl;

				// create the interfaces by making a bunch of squares, taking a screenshot, and saving

				darkTheme.close();
			}
			catch (const std::exception& e)
			{
				std::cerr << "failed to create dark theme directory" << std::endl;
				std::cerr << e.what() << std::endl;
			}
		}
	}

	if (!fs::exists(GBL::DIR::themes + "light/light.sfuitheme"))
	{
		if (!fs::exists(GBL::DIR::themes + "light"))
		{
			try
			{
				fs::create_directory(GBL::DIR::themes + "light");

				std::cout << "creating light theme file" << std::endl;
				std::ofstream lightTheme(GBL::DIR::themes + "light/light.sfuitheme");

				lightTheme << "// default 'light' theme for kunlauncher" << std::endl;
				lightTheme << std::endl;
				lightTheme << "name = light" << std::endl;
				lightTheme << "author = kennyrkun" << std::endl;
				lightTheme << "release = 1" << std::endl;
				lightTheme << "launcherrelease = 17" << std::endl;
				lightTheme << std::endl;
				lightTheme << "// globals" << std::endl;
				lightTheme << "primary = 150, 150, 150" << std::endl;
				lightTheme << "secondary = 180, 180, 180" << std::endl;
				lightTheme << "tertiary = 220, 220, 220" << std::endl;
				lightTheme << "text = 0, 0, 0" << std::endl;
				lightTheme << "text_secondary = 255, 255, 255" << std::endl;
				lightTheme << std::endl;
				lightTheme << "// scrollbar" << std::endl;
				lightTheme << "scrollbar = 241, 241, 241" << std::endl;
				lightTheme << "scrollthumb = 110, 110, 192" << std::endl;
				lightTheme << "scrollthumb_hover = 168, 168, 168" << std::endl;
				lightTheme << "scrollthumb_hold = 120, 120, 120" << std::endl;
				lightTheme << std::endl;
				lightTheme << "// apps" << std::endl;
				lightTheme << "app_card = 100, 100, 100" << std::endl;
				lightTheme << "app_card2 = 100, 100, 100" << std::endl;
				lightTheme << "app_image = 255, 255, 255" << std::endl;
				lightTheme << "app_icon = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_hover = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_press = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_fail = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_fail_hover = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_fail_press = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_warn = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_warn_hover = 255, 255, 255" << std::endl;
				lightTheme << "app_icon_warn_press = 255, 255, 255" << std::endl;

				lightTheme.close();
			}
			catch (const std::exception& e)
			{
				std::cerr << "failed to create light theme directory" << std::endl;
				std::cerr << e.what() << std::endl;
			}
		}
	}

	// FIXME: load themes
	/*
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

		//scrollTrack
		std::cout << "scrollTrack theme settings" << std::endl;

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

		// apps
		std::cout << "item theme settings" << std::endl;

		if (settings.get("item_card", colors))
		{
			GBL::COLOR::APP::CARD = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		if (settings.get("item_icon", colors))
		{
			GBL::COLOR::APP::ICON = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		if (settings.get("item_redownload", colors))
		{
			GBL::COLOR::APP::REDOWLOAD = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		if (settings.get("item_update_is_available", colors))
		{
			GBL::COLOR::APP::UPDATE_IS_AVAILABLE = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		if (settings.get("item_download", colors))
		{
			GBL::COLOR::APP::DOWNLOAD = sf::Color(colors[0], colors[1], colors[2]);
			colors.clear();
		}

		std::cout << "done applying themes" << std::endl;
	}
	else
	{
		std::cerr << "failed to load settings file" << std::endl; // use default colours
	}
	*/

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

	app->window->clear(sf::Color(50, 50, 50));
	app->window->draw(initialiseText);
	app->window->draw(currentLauncherTask);
	app->window->draw(thingthatdoesntripoffvisualstudio);
	app->window->draw(*progressBar);
	app->window->display();

//	std::cin.get();
}
