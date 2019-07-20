#include "AppEngine.hpp"
#include "AppListState.hpp"
#include "AppEditState.hpp"
#include "AppUploadState.hpp"

#include "Globals.hpp"
#include "Download.hpp"

#include <SFUI/SFUI.hpp>

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

enum MenuCallbacks
{
	BACK = -9999,
	RedownloadAppsList
};

void AppListState::Init(AppEngine* app_)
{
	std::cout << "AppListState Init" << std::endl;
	app = app_;

	populateApplist();

	std::cout << "creating menu" << std::endl;
	menu = new SFUI::Menu(*app->window);
	createMenu(*menu);

	std::cout << "AppListState ready." << std::endl;
}

void AppListState::Cleanup()
{
	std::cout << "Cleaning up AppListState" << std::endl;

	delete menu;

	std::cout << "Cleaned up AppListState." << std::endl;
}

void AppListState::Pause()
{
	std::cout << "AppListState paused" << std::endl;
}

void AppListState::Resume()
{
	std::cout << "AppListState resumed" << std::endl;
}

void AppListState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			std::cout << "quitting" << std::endl;

			app->Quit();
			return;
		}
		else if (event.type == sf::Event::EventType::Resized)
		{
			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				app->window->setView(sf::View(visibleArea));
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

		int id = menu->onEvent(event);
		switch (id)
		{
		case MenuCallbacks::RedownloadAppsList:
		{
			app->drawInformationPanel("Redownloading index...");

			redownloadAppsList();

			delete menu;
			menu = new SFUI::Menu(*app->window);
			createMenu(*menu);

			break;
		}
		case MenuCallbacks::BACK:
			app->PopState();
			return;
		}

		if (id > -1)
		{
			std::cout << "editing app " << id << std::endl;

			app->appToEdit = id;

			app->PushState(new AppEditState);
			return;
		}
	}
}

void AppListState::Update()
{
}

void AppListState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	app->window->draw(*menu);

	app->window->display();
}

void AppListState::createMenu(SFUI::Menu& menu)
{
	menu.setPosition(sf::Vector2f(10, 10));

	menu.addButton("Redownload App List", MenuCallbacks::RedownloadAppsList);

	for (const auto& [appid, appname] : applist)
	{
		SFUI::HorizontalBoxLayout* hbox = menu.addHorizontalBoxLayout();

		hbox->addButton("Edit " + std::to_string(appid), appid);
		hbox->addLabel(appname == "n0_aPp_nAm3" ? "App Has No Name" : appname);
	}

	menu.addButton("Back", MenuCallbacks::BACK);
}

//  FIXME: app names are only properly loaded the first time this is called
void AppListState::populateApplist()
{
	std::cout << "populating applist" << std::endl;

	applist.clear();

	std::ifstream readIndex(GBL::DIR::appcache + "index.dat", std::ios::in);

	SettingsParser indexParser;
	indexParser.loadFromFile(GBL::DIR::appcache + "index.dat");

	std::string line; // each line of index.dat
	int loopi(0);
	while (std::getline(readIndex, line))
	{
		// don't count this one as an app and stop crashing my damn program
		if (line.find("nextAppID = ") != std::string::npos)
		{
			std::cout << "not an app, skipping" << std::endl;
			continue;
		}

		int appid = std::stoi(line.substr(0, 1));

		std::cout << "iteration: " << loopi << ", appid: " << appid << std::endl;

		std::string item_name; // the name of the app
		if (indexParser.get(line.substr(0, 1), item_name))
		{
			applist.emplace(appid, getAppName(appid));

			if (!fs::exists(GBL::DIR::apps + std::to_string(appid)))
			{
				downloadApp(appid);
				std::cout << "downloaded info for app " << appid << std::endl;
			}
		}
		else
		{
			std::cerr << "some kind of error happened. line 197 in file AppListState.cpp" << std::endl;
		}

		loopi += 1;
	}

	readIndex.close();
}

void AppListState::redownloadAppsList()
{
	std::cout << "downloading all apps" << std::endl;

	sf::Clock appLoadTime;

	// TODO: check for index updates
	if (!app->updateAppIndex())
	{
		std::cerr << "failed to download appslist, giving up." << std::endl;
		return;
	}

	populateApplist();

	std::cout << "finished loading apps" << " (" << "in " << appLoadTime.getElapsedTime().asSeconds() << " seconds)" << std::endl;

	app->window->requestFocus();
}

std::string AppListState::getAppName(int appid) // a lot easier than I thought it would be.
{
	std::cout << "parsing info for " << appid << std::endl;

	std::string path = GBL::DIR::apps + std::to_string(appid) + "/";

	if (fs::exists(path))
	{
		SettingsParser itemInfoParser;
		if (itemInfoParser.loadFromFile(path + "info.dat"))
		{
			std::string appname;
			
			if (!itemInfoParser.get("name", appname))
				return "n0_aPp_nAm3";

			return appname;
		}
		else
		{
			std::cerr << "unable to load info for" << appid << std::endl;
		}
	}
	else
	{
		std::cerr << "info file for " << appid << " is missing" << std::endl;
	}

	return "n0_aPp_nAm3";
}

int AppListState::downloadApp(int appid)
{
	downloadInfo(appid);
	// do we actually need tese?
//	downloadIcon(appid);
//	downloadFiles(appid);
	return 0;
}

int AppListState::downloadIcon(int appid)
{
	std::cout << "\n" << "downloading icon" << std::endl;

	Download getIcon;
	getIcon.setInput(".//" + GBL::WEB::APPS + std::to_string(appid) + "//icon.png");
	getIcon.setOutputDir(GBL::DIR::apps + std::to_string(appid) + "//");
	getIcon.setOutputFilename("icon.png");

	int status = getIcon.download();

	if (status == Download::Status::Ok)
		return getIcon.save();
	else
	{
		std::cout << "failed to download app icon, not saving file" << std::endl;
		return 0;
	}
}

int AppListState::downloadInfo(int appid)
{
	std::cout << "\n" << "downloading info" << std::endl;

	Download getInfo;
	getInfo.setInput(GBL::WEB::APPS + std::to_string(appid) + "/info.dat");
	getInfo.setOutputDir(GBL::DIR::apps + std::to_string(appid) + "//");
	getInfo.setOutputFilename("info.dat");

	int status = getInfo.download();

	if (status == Download::Status::Ok)
		return getInfo.save();
	else
	{
		std::cout << "failed to download app info, not saving file" << std::endl;
		return 0;
	}
}

int AppListState::downloadFiles(int appid)
{
	std::cout << "\n" << "downloading files" << std::endl;

	Download getFiles;
	getFiles.setInput(GBL::WEB::APPS + std::to_string(appid) + "/release.zip");
	getFiles.setOutputDir(GBL::DIR::apps + std::to_string(appid) + "//");
	getFiles.setOutputFilename("release.zip");

	int status = getFiles.download();

	if (status == Download::Status::Ok)
		return getFiles.save();
	else
	{
		std::cout << "failed to download app info, not saving file" << std::endl;
		return 0;
	}
}

bool AppListState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

bool AppListState::mouseIsOver(sf::Shape &object, sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool AppListState::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}
