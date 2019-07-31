#include "MyApp.hpp"

#include "Download.hpp"
#include "Globals.hpp"
#include "MessageBox.hpp"
#include "SettingsParser.hpp"

#include <SFML/Network.hpp>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>
#include <future>

namespace fs = std::experimental::filesystem;

MyApp::MyApp(int appid, float xSize, float ySize, float xPos, float yPos)
{
	sf::Clock itemCreateTimer;

	std::cout << "creating new card for \"" + std::to_string(appid) + "\"" << std::endl;

	info.appid = appid;
	itemInstallDir = GBL::DIR::apps + std::to_string(info.appid) + "/"; // .//bin//apps//itemName//

	if (!fs::exists(itemInstallDir))
	{
		std::cout << "app directory does not already exist, creating..." << std::endl;

		try
		{
			fs::create_directory(itemInstallDir);
		}
		catch (const std::exception& e)
		{
			std::cerr << "failed to create directory" << std::endl;
			std::cerr << e.what() << std::endl;
		}
	}

	if (fs::exists(itemInstallDir + "icon.png"))
	{
		std::cout << "loaded icon" << std::endl;

		iconTexture.loadFromFile(itemInstallDir + "icon.png");
	}
	else // icon is not downloaded
	{
		std::cout << "icon was not found" << std::endl;

		// TODO: try to find the icon

		iconTexture.loadFromFile(GBL::DIR::textures + "missing_icon_icon.png");
	}

	if (fs::exists(itemInstallDir + "info.dat"))
	{
		std::cout << "info was found, parsing" << std::endl;
	}
	else // info is not downloaded
	{
		std::cout << "info was not found" << std::endl;
	}
	parseInfo(itemInstallDir);

	if (fs::exists(itemInstallDir + "release.zip"))
	{
		std::cout << "release was found, installed" << std::endl;

		info.downloaded = true;

		//		checkForUpdate();
	}
	else // thing is not downloaded
	{
		std::cout << "release was not found, not installed" << std::endl;

		info.downloaded = false;
	}

	cardShape.setSize(sf::Vector2f(xSize, ySize));
	cardShape.setPosition(sf::Vector2f(xPos, yPos)); // probably not the best
	cardShape.setFillColor(GBL::theme.palatte.APP_CARD);

	icon.setSize(sf::Vector2f(cardShape.getSize().y, cardShape.getSize().y)); // a square
	icon.setPosition(cardShape.getPosition());
	icon.setTexture(&iconTexture);
	iconTexture.setSmooth(true);

	font = *GBL::theme.getFont("Arial.ttf");

	name.setFont(font);
	name.setCharacterSize(24);
	name.setFillColor(GBL::theme.palatte.TEXT_SECONDARY);

	description.setFont(font);
	description.setCharacterSize(16);
	description.setFillColor(GBL::theme.palatte.TEXT_SECONDARY);

	version.setFont(font);
	version.setCharacterSize(18);
	version.setFillColor(GBL::theme.palatte.TEXT_SECONDARY);

	downloadButton.setFillColor(GBL::theme.palatte.APP_ICON);
	downloadButton.setTexture(GBL::theme.getTexture("get_app_1x.png"));
	downloadButton.setSize(sf::Vector2f(24, 24));

	redownloadButton.setFillColor(GBL::theme.palatte.APP_ICON);
	redownloadButton.setTexture(GBL::theme.getTexture("auto_renew_1x.png"));
	redownloadButton.setRadius(10);
	redownloadButton.setRotation(30);

	removeButton.setFillColor(GBL::theme.palatte.APP_ICON);
	removeButton.setTexture(GBL::theme.getTexture("delete_forever_1x.png"));
	removeButton.setSize(sf::Vector2f(24, 24));

	launchButton.setTexture(GBL::theme.getTexture("launch_1x.png"));
	launchButton.setSize(sf::Vector2f(20, 20));
	
	updateSizeAndPosition(xSize, ySize, xPos, yPos);

	std::cout << "card is ready (took " << itemCreateTimer.getElapsedTime().asSeconds() << " seconds)" << std::endl;
}

MyApp::~MyApp()
{
	// don't delete targetwindow because it's just a pointer and if we delete it it's actually deleting the app's renderwindow
	// and i spent all day debugging this one problem but it was just the destructor and wowiee fuck me
}

void MyApp::setPosition(const sf::Vector2f& pos)
{
	updateSizeAndPosition(cardShape.getSize().x, cardShape.getSize().y, pos.x, pos.y);
}

int MyApp::onClick(sf::Vector2f clickPos)
{
	if (info.downloaded)
	{
		if (mouseIsOver(removeButton, clickPos))
		{
			if (deleteFilesPrompt())
			{
				return CLICKBACK::DeleteFiles;
			}
		}
		else if (mouseIsOver(redownloadButton, clickPos))
		{
			std::atomic<bool> done(false);

			AsyncTask* tt = new AsyncTask;
			tt->future = std::async(std::launch::async, &MyApp::redownload, this);
			GBL::threadManager.addTask(tt);
		}
		else if (mouseIsOver(launchButton, clickPos))
		{
			return CLICKBACK::OpenFiles;
		}
	}
	else
	{
		if (mouseIsOver(downloadButton, clickPos))
		{
			std::cerr << "download button pressed" << std::endl;
		}
	}

	return None;
}

bool MyApp::deleteFilesPrompt()
{
	MessageBox::Options modOptions = { "Confirm Deletion", "Delete " + info.name + "?", { "No", "Yes" } };

	MessageBox confirmDelete(modOptions);
	confirmDelete.runBlocking();

	if (confirmDelete.exitCode == 1)
		return true;

	return false;
}

void MyApp::deleteFiles()
{
	std::cout << "deleting app files" << std::endl;

	try
	{
		sf::Texture temp;
		iconTexture = temp;

		fs::remove_all(itemInstallDir);
		std::cout << "done" << std::endl;
		info.downloaded = false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "\n" << "failed to delete files: " << std::endl;
		std::cerr << "\n" << e.what() << std::endl;
	}
}

bool MyApp::checkForUpdate(sf::Ftp& ftp)
{
	std::cout << "checking for updates" << std::endl;

	sf::Ftp::Response response = ftp.sendCommand("SIZE", GBL::WEB::APPS + std::to_string(info.appid) + "/release.zip");
	if (response.isOk())
	{
		size_t remoteFileSize = std::stoi(response.getMessage());
		size_t fileSize = fs::file_size(GBL::DIR::apps + std::to_string(info.appid) + "/release.zip");

		if (fileSize != remoteFileSize)
			return true;
	}
	else
		std::cerr << response.getMessage() << std::endl;

	return false;
}

void MyApp::redownload()
{
	std::cout << "updating item" << std::endl;

	download();
}

void MyApp::download()
{
	bool failure = false;

	info.downloading = true;

	float fuckedUpXPosition = (cardShape.getPosition().x + cardShape.getLocalBounds().width) - 30;
	redownloadButton.setOrigin(sf::Vector2f(redownloadButton.getLocalBounds().width / 2, redownloadButton.getLocalBounds().height / 2));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 37.f));
	info.missingInfo = true;

	std::cout << "downloading " << info.name << std::endl;

	if (!downloadInfo())
		failure = true;

	if (!downloadIcon())
		failure = true;
	else
		iconTexture.loadFromFile(itemInstallDir + "icon.png");

	if (!downloadFiles())
		failure = true;

	info.downloading = false;

	if (!failure)
	{
		info.updateAvailable = false;

		redownloadButton.setFillColor(sf::Color::White);
		redownloadButton.setRotation(30);
		redownloadButton.setOrigin(sf::Vector2f(0, 0));
		redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition + 7, cardShape.getPosition().y + 10));
		redownloadButtonTexture.loadFromFile(GBL::DIR::textures + "auto_renew_1x.png");
	}
	else
	{
		std::cout << "download failed" << std::endl;
		redownloadButton.setFillColor(sf::Color::Red);
	}
}

void MyApp::openItem()
{
	//TODO: proper system for opening and monitoring apps

#ifdef _WIN32
	std::cout << "opening item" << std::endl;
	std::string launch = "start " + itemInstallDir + "release.zip -withkunlauncher";
	system(launch.c_str());
#else
	GBL::MESSAGES::cantOpenNotWindows();
#endif
}

void MyApp::updateReady()
{
	info.updateAvailable = true;

	redownloadButton.setFillColor(sf::Color::Yellow);
	redownloadButton.setRotation(0);
	redownloadButtonTexture.loadFromFile(GBL::DIR::textures + "error_1x.png");
	redownloadButton.setTexture(&redownloadButtonTexture, true);
}

void MyApp::updateSizeAndPosition(float xSize, float ySize, float xPos, float yPos)
{
	cardShape.setPosition(sf::Vector2f(xPos, yPos)); // probably not the best
	cardShape.setSize(sf::Vector2f(xSize, ySize));

	icon.setPosition(sf::Vector2f(cardShape.getPosition()));

	const int isize = icon.getSize().x;
	name.setPosition(static_cast<int>(icon.getPosition().x + isize + 10), static_cast<int>(cardShape.getPosition().y));
	description.setPosition(static_cast<int>(icon.getPosition().x + isize + 10), static_cast<int>(cardShape.getPosition().y + 25));
	version.setPosition(static_cast<int>(icon.getPosition().x + isize + 10), static_cast<int>(cardShape.getPosition().y + 50));

	float fuckedUpXPosition = (cardShape.getPosition().x + cardShape.getLocalBounds().width) - 30;

	downloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 10));

	redownloadButton.setRotation(30);
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition + 7, cardShape.getPosition().y + 10));

	removeButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 41));

	launchButton.setPosition(sf::Vector2f(fuckedUpXPosition - 28, 
										  cardShape.getPosition().y + 27.5f));
}

void MyApp::update()
{
	if (info.downloading)
		redownloadButton.rotate(1);
}

void MyApp::draw(sf::RenderTarget& target, sf::RenderStates states) const 
{
	target.draw(cardShape, states);

	target.draw(icon, states);

	target.draw(name, states);
	target.draw(description, states);
	target.draw(version, states);

	if (info.downloading)
		target.draw(redownloadButton, states);
	else // not downloading
		if (info.downloaded)
		{
			target.draw(redownloadButton, states);
			target.draw(removeButton, states);
			target.draw(launchButton, states);
		}
		else // not downloaded
			if (!info.missingInfo) // not missing info
				target.draw(downloadButton, states);
}

// private

void MyApp::parseInfo(std::string dir) // a lot easier than I thought it would be.
{
	//TODO: add checks here to make sure we don't try to parse a file that ended up being a 500 or 404

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

			if (!info.description.empty())
				description.setString(info.description);
			else
			{
				description.setStyle(sf::Text::Style::Italic);
				description.setString("No description provided.");
			}

			if (info.version > 0) // less than zero and it's fucked
				version.setString(std::to_string(info.version));
			else
			{
				version.setStyle(sf::Text::Style::Italic);
				version.setString("Unknown");
			}
		}
		else
		{
			name.setString("Unable to load item info!");
			description.setString("Try redownloading the app.");
			version.setString("PARSE-" + std::to_string(info.appid));
		}
	}
	else
	{
		std::cerr << "info file is empty or missing" << std::endl;

		updateReady();

		iconTexture.loadFromFile(GBL::DIR::textures + "error_2x.png");
		icon.setTexture(&iconTexture, true);

		name.setString("missing info for \"" + std::to_string(info.appid) + "\"");
		description.setString("missing info.dat; try redownloading");

		info.missingInfo = true;
	}
}

int MyApp::downloadIcon()
{
	std::cout << "\n" << "downloading icon" << std::endl;

	Download getIcon;
	getIcon.setInput(".//" + GBL::WEB::APPS + std::to_string(info.appid) + "//icon.png");
	getIcon.setOutputDir(GBL::DIR::apps + std::to_string(info.appid) + "//");
	getIcon.setOutputFilename("icon.png");
	getIcon.download();
	getIcon.save();

	// TODO: replace this with a check in getIcon.download()
	if (fs::exists(itemInstallDir + "icon.png"))
	{
		iconTexture.loadFromFile(itemInstallDir + "icon.png");
		icon.setTexture(&iconTexture, true);

		return 1;
	}
	else
	{
		return 0;
	}
}

int MyApp::downloadInfo()
{
	std::cout << "\n" << "downloading info" << std::endl;

	Download getInfo;
	getInfo.setInput(GBL::WEB::APPS + std::to_string(info.appid) + "/info.dat");
	getInfo.setOutputDir(GBL::DIR::apps + std::to_string(info.appid) + "//");
	getInfo.setOutputFilename("info.dat");
	getInfo.download();
	getInfo.save();

	parseInfo(itemInstallDir);

	return 1;
}

int MyApp::downloadFiles()
{
	std::cout << "\n" << "downloading files" << std::endl;

	std::string appPath = GBL::DIR::apps + std::to_string(info.appid) + "/";

	Download getFiles;
	getFiles.setInput(GBL::WEB::APPS + std::to_string(info.appid) + "/release.zip");
	getFiles.setOutputDir(appPath);
	getFiles.setOutputFilename("release.zip");

	if (getFiles.download() == Download::Status::Success)
	{
		if (fs::exists(appPath + "release.zip"))
		{
			try
			{
				fs::remove(appPath + "release.zip");
			}
			catch (const fs::filesystem_error& e)
			{
				std::cerr << "failed to remove old release files:" << std::endl;
				std::cerr << e.what() << std::endl;
				return -1;
			}
		}

		info.downloaded = true;
		getFiles.save();
		return 1;
	}
	else
	{
		return -1;
	}
}
