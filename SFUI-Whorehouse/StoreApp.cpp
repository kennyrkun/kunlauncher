#include "StoreApp.hpp"

#include "Download.hpp"
#include "Globals.hpp"
#include "MessageBox.hpp"
#include "SettingsParser.hpp"

#include <SFML/Network.hpp>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

// used to keep source code clean and short
#define ICON_X_POS (cardShape.getPosition().x + cardShape.getLocalBounds().width) - 28
#define ICON_Y_POS (cardShape.getPosition().y + cardShape.getLocalBounds().height / 2)

// TODO: remove origin from icons
// TODO: call update size and position in here
StoreApp::StoreApp(int appid, float xSize, float ySize, const sf::Vector2f& position)
{
	sf::Clock itemCreateTimer;

	std::cout << "creating new card for \"" + std::to_string(appid) + "\"" << std::endl;

	info.appid = appid;
	// where the item info and icon are stored
	itemCacheDir = GBL::DIR::appcache + std::to_string(info.appid) + "/"; // ./bin/appcache/appid/
	// where the item info and icon are moved when it's downloaded
	itemInstallDir = GBL::DIR::apps + std::to_string(info.appid) + "/"; // ./bin/apps/appid/

	if (!fs::exists(itemCacheDir))
	{
		std::cout << "appcache directory does not already exist, creating..." << std::endl;

		try
		{
			fs::create_directory(itemCacheDir);
		}
		catch (const std::exception& e)
		{
			std::cerr << "failed to create directory" << std::endl;
			std::cerr << e.what() << std::endl;
		}
	}

	if (!fs::exists(itemCacheDir + "info.dat"))
	{
		std::cout << "info was not found, downloading" << std::endl;

		downloadInfo();
	}
	parseInfo(itemCacheDir);

	if (fs::exists(itemCacheDir + "icon.png"))
	{
		std::cout << "icon was found" << std::endl;

		iconTexture.loadFromFile(itemCacheDir + "icon.png");
	}
	else // icon is not downloaded
	{
		std::cout << "icon was not found, downloading" << std::endl;

		if (downloadIcon())
			iconTexture.loadFromFile(itemCacheDir + "icon.png");
		else
			iconTexture = *GBL::theme.getTexture("missing_icon_icon.png");
	}

	if (fs::exists(itemInstallDir + "release.zip"))
	{
		std::cout << "release was found, installed" << std::endl;

		info.status.downloaded = true;

		// TODO: check for update
//		checkForUpdate();
	}
	else // thing is not downloaded
	{
		std::cout << "release was not found, not installed" << std::endl;

		info.status.downloaded = false;
	}

	cardShape.setSize(sf::Vector2f(xSize, ySize));
	cardShape.setPosition(sf::Vector2f(position.x, position.y)); // probably not the best
	cardShape.setFillColor(GBL::theme.palatte.APP_CARD);

	icon.setSize(sf::Vector2f(cardShape.getSize().y, cardShape.getSize().y)); // a square
	icon.setFillColor(GBL::theme.palatte.APP_IMAGE);
	icon.setTexture(&iconTexture);
	iconTexture.setSmooth(true);

	name.setFont(*GBL::theme.getFont("Arial.ttf"));
	name.setCharacterSize(24);
	name.setFillColor(GBL::theme.palatte.TEXT_SECONDARY);

	infoButton.setFillColor(GBL::theme.palatte.APP_ICON);
	infoButton.setRadius(12);
	infoButton.setTexture(GBL::theme.getTexture("info_1x.png"));

	openInMyAppsListButton.setFillColor(GBL::theme.palatte.APP_ICON);
	openInMyAppsListButton.setSize(sf::Vector2f(24, 24));
	openInMyAppsListButton.setTexture(GBL::theme.getTexture("input_1x.png"));

	updateSizeAndPosition(xSize, ySize, position.x, position.y);

	std::cout << "card is ready (took " << itemCreateTimer.getElapsedTime().asSeconds() << " seconds)" << std::endl;
}

StoreApp::~StoreApp()
{
}

void StoreApp::setPosition(const sf::Vector2f& pos)
{
	updateSizeAndPosition(cardShape.getSize().x, cardShape.getSize().y, pos.x, pos.y);
}

int StoreApp::onClick(sf::Vector2f clickPos)
{
	if (info.status.downloading)
		return -1;
	else if (mouseIsOver(infoButton, clickPos))
	{
		if (info.status.redownloadRequired)
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
				infoPanel.open(info);
			else
			{
				AsyncTask* tt = new AsyncTask;
				tt->future = std::async(std::launch::async, &StoreApp::download, this);
				GBL::threadManager.addTask(tt);
			}
		}
		else
			infoPanel.open(info);
	}
	else if (mouseIsOver(openInMyAppsListButton, clickPos))
	{
		if (info.status.downloaded)
		{
			// TODO: open item in myappslist
			openItem();
		}
	}

	return -1;
}

bool StoreApp::checkForUpdate(sf::Ftp& ftp)
{
	if (!info.status.downloaded)
		return false;

	if (App::checkForUpdate(ftp))
	{
		info.status.updateAvailable = true;

		infoButton.setFillColor(sf::Color::Yellow);

		return true;
	}

	return false;
}

bool StoreApp::deleteFilesPrompt()
{
	MessageBox::Options modOptions = { "Confirm Deletion", "Delete " + info.name + "?", { "No", "Yes" } };

	MessageBox confirmDelete(modOptions);
	confirmDelete.runBlocking();

	if (confirmDelete.exitCode == 1)
		return true;

	return false;
}

void StoreApp::deleteFiles()
{
	std::cout << "deleting app files" << std::endl;

	try
	{
		fs::remove_all(itemInstallDir);

		std::cout << "done" << std::endl;
		info.status.downloaded = false;
		infoPanel.updateMenu(info);
	}
	catch (const std::exception& e)
	{
		std::cerr << "failed to delete files: " << std::endl;
		std::cerr << e.what() << std::endl;
	}
}

void StoreApp::download()
{
	info.status.redownloadRequired = false; // because we're downloading, it is no longer required

	float fuckedUpXPosition = (cardShape.getPosition().x + cardShape.getLocalBounds().width) - 30;
	infoButton.setTexture(GBL::theme.getTexture("auto_renew_1x.png"));

	info.status.downloading = true;

	if (infoPanel.isOpen())
		infoPanel.updateMenu(info);

	std::cout << "downloading " << info.name << std::endl;

	if (!downloadInfo())
	{
		std::cerr << "failed to download info" << std::endl;
		info.status.redownloadRequired = true;
	}
	else
	{
		std::cout << "downloaded info success" << std::endl;
	}

	if (!downloadIcon())
	{
		std::cerr << "failed to download icon" << std::endl;
		info.status.redownloadRequired = true;
	}
	else
	{
		iconTexture.loadFromFile(itemInstallDir + "icon.png");
		std::cout << "downloaded icon success" << std::endl;
	}

	if (!downloadFiles())
	{
		std::cerr << "failed to download info" << std::endl;
		info.status.redownloadRequired = true;
	}
	else
	{
		std::cout << "downloaded files success" << std::endl;
	}

	info.status.downloading = false;

	if (info.status.redownloadRequired)
	{
		std::cerr << "download failed" << std::endl;
		infoButton.setRotation(0);
		infoButton.setFillColor(sf::Color::Red);
		infoButton.setTexture(GBL::theme.getTexture("refresh_1x.png"));
	}
	else
	{
		info.status.updateAvailable = false;
		info.status.redownloadRequired = false;
		info.status.downloaded = true;

		infoButton.setRotation(0);
		infoButton.setFillColor(sf::Color::White);
		infoButton.setTexture(GBL::theme.getTexture("info_1x.png"));
	}


	std::cout << "finished downloading " << info.name << std::endl;

	if (infoPanel.isOpen())
		infoPanel.updateMenu(info);
}

void StoreApp::openItem()
{
	//TODO: proper system for opening and monitoring apps

#ifdef _WIN32
	std::cout << "opening item" << std::endl;
	std::string launch = "start " + itemInstallDir + "release.zip -kunlaunched";
	system(launch.c_str());
#else
	GBL::MESSAGES::cantOpenNotWindows();
#endif
}

void StoreApp::updateSizeAndPosition(float xSize, float ySize, float xPos, float yPos)
{
	cardShape.setPosition(sf::Vector2f(xPos, yPos));
	cardShape.setSize(sf::Vector2f(xSize, ySize));

	icon.setPosition(cardShape.getPosition());

	name.setPosition(static_cast<int>(icon.getPosition().x + icon.getSize().x + 10.0f), static_cast<int>(cardShape.getPosition().y + 10.0f));

	infoButton.setOrigin(sf::Vector2f(infoButton.getLocalBounds().width / 2, infoButton.getLocalBounds().height / 2));
	infoButton.setPosition(sf::Vector2f(ICON_X_POS, ICON_Y_POS));

	openInMyAppsListButton.setOrigin(sf::Vector2f(openInMyAppsListButton.getLocalBounds().width / 2, openInMyAppsListButton.getLocalBounds().height / 2));
	openInMyAppsListButton.setPosition(sf::Vector2f(ICON_X_POS - 28, ICON_Y_POS));
}

// infopanel interactions are done here
void StoreApp::update()
{
	if (infoPanel.isOpen())
	{
		infoPanel.HandleEvents();
		infoPanel.Update();

		for (size_t i = 0; i < infoPanel.eventQueue.size(); i++)
		{
			if (infoPanel.eventQueue.back() == infoPanel.Delete)
			{
				if (deleteFilesPrompt())
				{
					AsyncTask* tt = new AsyncTask;
					tt->future = std::async(std::launch::async, &StoreApp::deleteFiles, this);
					GBL::threadManager.addTask(tt);
				}
			}
			else if (infoPanel.eventQueue.back() == infoPanel.Download ||
					 infoPanel.eventQueue.back() == infoPanel.Redownload ||
					 infoPanel.eventQueue.back() == infoPanel.UpdateApp)
			{
				if (!info.status.downloading)
				{
					AsyncTask* tt = new AsyncTask;
					tt->future = std::async(std::launch::async, &StoreApp::download, this);
					GBL::threadManager.addTask(tt);
				}
			}
		}
	}

	if (info.status.downloading)
		infoButton.rotate(1);
}

void StoreApp::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(cardShape, states);
	//	targetWindow->draw(controlBar);
	target.draw(icon, states);

	target.draw(name, states);

	target.draw(infoButton, states);

	if (info.status.downloaded)
		target.draw(openInMyAppsListButton, states);
}

// private

void StoreApp::parseInfo(std::string dir)
{
	std::cout << "parsing info for " << dir << std::endl;

	if (fs::exists(dir + "info.dat"))
	{
		SettingsParser itemInfo;
		if (itemInfo.loadFromFile(dir + "info.dat"))
		{
			itemInfo.get("name", info.name);
			itemInfo.get("description", info.description);
			if (!itemInfo.get("version", info.version))
				info.version = -1;
			itemInfo.get("author", info.author);
			itemInfo.get("github", info.github);
			itemInfo.get("release", info.release);
			
			if (!info.name.empty())
				name.setString(info.name);
			else
			{
				name.setStyle(sf::Text::Style::Italic);
				name.setString("appid(" + std::to_string(info.appid) + ")");
			}
		}
		else
		{
			std::cerr << "failed to load item info" << std::endl;
			name.setString("Unable to load item info.");
		}
	}
	else
	{
		std::cerr << "info file is empty or missing" << std::endl;

		iconTexture.loadFromFile(GBL::DIR::textures + "error_2x.png");

		name.setString("missing info for \"" + info.name + "\"");
		info.status.missinginfo = true;
	}
}

bool StoreApp::downloadIcon()
{
	Download getIcon;
	getIcon.setInput("./" + GBL::WEB::APPS + std::to_string(info.appid) + "/icon.png");
	getIcon.setOutputDir(itemCacheDir);
	getIcon.setOutputFilename("icon.png");

	int status = getIcon.download();

	if (status == Download::Status::Success)
	{
		getIcon.save();
		return true;
	}
	else
	{
		std::cerr << "failed to download icon: " << status << std::endl;
		return false;
	}
}

bool StoreApp::downloadInfo()
{
	std::cout << "\n" << "downloading info" << std::endl;

	Download getInfo;
	getInfo.setInput("./" + GBL::WEB::APPS + std::to_string(info.appid) + "/info.dat");
	getInfo.setOutputDir(itemCacheDir);
	getInfo.setOutputFilename("info.dat");

	int status = getInfo.download();
	
	if (status == Download::Status::Success)
	{
		getInfo.save();
		return true;
	}
	else
	{
		std::cerr << "failed to download info: " << status << std::endl;
		return false;
	}

}

bool StoreApp::downloadFiles()
{
	std::cout << "\n" << "downloading item" << std::endl;

	try
	{
		fs::remove_all(itemInstallDir);
		fs::create_directory(itemInstallDir);
		fs::copy(itemCacheDir, itemInstallDir);
	}
	catch (const std::exception& e)
	{
		std::cerr << "failed to copy cached app into apps directory, the files will be downloaded directely into the apps directory:" << std::endl;
		std::cerr << e.what() << std::endl;

		Download getIcon;
		getIcon.setInput("./" + GBL::WEB::APPS + std::to_string(info.appid) + "/icon.png");
		getIcon.setOutputDir(GBL::DIR::apps + std::to_string(info.appid) + "/");
		getIcon.setOutputFilename("icon.png");

		int iconStatus = getIcon.download();

		if (iconStatus != Download::Status::Success)
		{
			std::cerr << "failed to download icon: " << iconStatus << std::endl;
			return false;
		}

		getIcon.save();

		Download getInfo;
		getInfo.setInput("./" + GBL::WEB::APPS + std::to_string(info.appid) + "/info.dat");
		getInfo.setOutputDir(GBL::DIR::apps + std::to_string(info.appid) + "/");
		getInfo.setOutputFilename("info.dat");

		int infoStatus = getInfo.download();

		if (infoStatus != Download::Status::Success)
		{
			std::cerr << "failed to download info: " << infoStatus << std::endl;
			return false;
		}

		getInfo.save();

		parseInfo(itemInstallDir);
	}

	std::cout << "\n" << "downloading files" << std::endl;

	std::string appPath = GBL::DIR::apps + std::to_string(info.appid) + "/";

	Download getFiles;
	getFiles.setInput(GBL::WEB::APPS + std::to_string(info.appid) + "/release.zip");
	getFiles.setOutputDir(appPath);
	getFiles.setOutputFilename("release.zip");

	int status = getFiles.download();

	if (status == Download::Status::Success)
	{
		// if the files already exist, try to remove them
		if (fs::exists(appPath + "release.zip"))
		{
			try
			{
				fs::remove(appPath + "release.zip");
			}
			catch (const fs::filesystem_error& e)
			{
				// if we can't remove them, give up and die
				std::cerr << "failed to remove old release files:" << std::endl;
				std::cerr << e.what() << std::endl;
				return false;
			}
		}

		if (getFiles.save())
		{
			info.status.downloaded = true;
			return true;
		}
		else
		{
			std::cout << "failed to save files" << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "failed to download files: " << status << std::endl;
		return false;
	}
}

int StoreApp::install()
{
	try
	{
		fs::copy(itemCacheDir, itemInstallDir);
	}
	catch (const std::exception& e)
	{
		std::cerr << "failed to copy info and files, downloading direct." << std::endl;
		std::cerr << e.what() << std::endl;

		Download getIcon;
		getIcon.setInput("./" + GBL::WEB::APPS + std::to_string(info.appid) + "/icon.png");
		getIcon.setOutputDir(GBL::DIR::apps + std::to_string(info.appid) + "/");
		getIcon.setOutputFilename("icon.png");
		getIcon.download();
		getIcon.save();

		Download getInfo;
		getInfo.setInput("./" + GBL::WEB::APPS + std::to_string(info.appid) + "/info.dat");
		getInfo.setOutputDir(GBL::DIR::apps + std::to_string(info.appid) + "/");
		getInfo.setOutputFilename("info.dat");
		getInfo.download();
		getInfo.save();

		parseInfo(itemInstallDir);
	}

	return 0;
}
