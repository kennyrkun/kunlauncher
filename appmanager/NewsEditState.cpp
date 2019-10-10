#include "AppEngine.hpp"
#include "NewsEditState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "../SFUI-Whorehouse/SettingsParser.hpp"

#include <SFUI/SFUI.hpp>

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

enum MenuCallbacks
{
	BACK = -9999,
	RedownloadNewsList
};

void NewsEditState::Init(AppEngine* app_)
{
	std::cout << "NewsEditState Init" << std::endl;
	app = app_;

	populateApplist();

	std::cout << "creating menu" << std::endl;
	menu = new SFUI::Menu(*app->window);
	createMenu(*menu);

	std::cout << "NewsEditState ready." << std::endl;
}

void NewsEditState::Cleanup()
{
	std::cout << "Cleaning up NewsEditState" << std::endl;

	delete menu;

	std::cout << "Cleaned up NewsEditState." << std::endl;
}

void NewsEditState::Pause()
{
	std::cout << "NewsEditState paused" << std::endl;
}

void NewsEditState::Resume()
{
	std::cout << "NewsEditState resumed" << std::endl;
}

void NewsEditState::HandleEvents()
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
				sf::FloatRect visibleArea(0.0f, 0.0f, event.size.width, event.size.height);
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
		case MenuCallbacks::RedownloadNewsList:
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
			std::cout << "editing news " << newsList[id] << std::endl;

			app->appToEdit = id;

			//app->PushState(new AppEditState);
			return;
		}
	}
}

void NewsEditState::Update()
{
}

void NewsEditState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	app->window->draw(*menu);

	app->window->display();
}

void NewsEditState::createMenu(SFUI::Menu& menu)
{
	menu.setPosition(sf::Vector2f(10, 10));

	menu.addButton("Redownload App List", MenuCallbacks::RedownloadNewsList);

	for (size_t i = 0; i < newsList.size(); i++)
		menu.addButton(newsList[i], i);

	menu.addButton("Back", MenuCallbacks::BACK);
}

//  FIXME: app names are only properly loaded the first time this is called
void NewsEditState::populateApplist()
{
	std::cout << "populating applist" << std::endl;

	newsList.clear();

	Download getNews;
	getNews.setInput("./" + GBL::WEB::NEWS + "/news.txt");
	getNews.setOutputDir(GBL::DIR::installDir);
	getNews.setOutputFilename("/news.txt");

	if (getNews.download() == Download::Status::Ok)
	{
		getNews.save();
	}
	else
	{
		std::cerr << "failed to download news" << std::endl;
		abort();
	}

	std::ifstream readIndex(GBL::DIR::installDir + "news.txt", std::ios::in);

	std::string line; // each line of index.dat
	bool nextLineIsNewLine = true; // starts as true because the first line is a new one
	while (std::getline(readIndex, line))
	{
		std::cout << std::endl;

		if (nextLineIsNewLine)
		{
			newsList.push_back(line);
			nextLineIsNewLine = false;
		}
		else
			if (line == "-----------------------------")
				nextLineIsNewLine = true;
	}

	readIndex.close();

	std::cout << std::endl;
}

void NewsEditState::redownloadAppsList()
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

std::string NewsEditState::getAppName(int appid) // a lot easier than I thought it would be.
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

int NewsEditState::downloadApp(int appid)
{
	downloadInfo(appid);
	// do we actually need tese?
//	downloadIcon(appid);
//	downloadFiles(appid);
	return 0;
}

int NewsEditState::downloadIcon(int appid)
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

int NewsEditState::downloadInfo(int appid)
{
	std::cout << "downloading info" << std::endl;

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

int NewsEditState::downloadFiles(int appid)
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

bool NewsEditState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

bool NewsEditState::mouseIsOver(sf::Shape &object, sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool NewsEditState::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}
