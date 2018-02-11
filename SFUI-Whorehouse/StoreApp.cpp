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

StoreApp::StoreApp(std::string itemName_, sf::RenderWindow* target_window, float xSize, float ySize, float xPos, float yPos)
{
	sf::Clock itemCreateTimer;

	std::cout << "creating new card for \"" + itemName_ + "\"" << std::endl;

	targetWindow = target_window;
	info.name = itemName_;
	itemInstallDir = GBL::DIR::apps + info.name + "//"; // .//bin//apps//itemName//

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
		std::cout << "icon was found" << std::endl;

		iconTexture.loadFromFile(itemInstallDir + "icon.png");
	}
	else // icon is not downloaded
	{
		std::cout << "icon was not found, downloading" << std::endl;

		downloadIcon();
	}

	if (fs::exists(itemInstallDir + "info.dat"))
	{
		std::cout << "info was found, parsing" << std::endl;
	}
	else // info is not downloaded
	{
		std::cout << "info was not found, downloading" << std::endl;
		downloadInfo();
	}
	parseInfo(itemInstallDir);

	if (fs::exists(itemInstallDir + "release.zip"))
	{
		std::cout << "release was found, installed" << std::endl;

		downloaded = true;

		checkForUpdate();
	}
	else // thing is not downloaded
	{
		std::cout << "release was not found, not installed" << std::endl;

		downloaded = false;
	}

	cardShape.setSize(sf::Vector2f(xSize, 75));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f(xPos, yPos)); // probably not the best
	cardShape.setFillColor(GBL::COLOR::ITEM::CARD);

	totalHeight = cardShape.getSize().y;

	icon.setSize(sf::Vector2f(cardShape.getSize().y, cardShape.getSize().y)); // a square
	icon.setOrigin(sf::Vector2f(icon.getLocalBounds().width / 2, icon.getLocalBounds().height / 2));
	icon.setPosition(sf::Vector2f(cardShape.getPosition().x - (cardShape.getSize().x / 2) + icon.getLocalBounds().width / 2, cardShape.getPosition().y));
	icon.setTexture(&iconTexture);
	iconTexture.setSmooth(true);
	
	font.loadFromFile(GBL::DIR::fonts + "Arial.ttf");

	name.setFont(font);
	description.setFont(font);
	version.setFont(font);

	name.setCharacterSize(24);
	description.setCharacterSize(16);
	version.setCharacterSize(18);

	name.setPosition(static_cast<int>(icon.getPosition().x + 45), static_cast<int>(cardShape.getPosition().y - 40));
	description.setPosition(static_cast<int>(icon.getPosition().x + 45), static_cast<int>(cardShape.getPosition().y - 15));
	version.setPosition(static_cast<int>(icon.getPosition().x + 45), static_cast<int>(cardShape.getPosition().y + 10));

	name.setFillColor(GBL::COLOR::TEXT);
	description.setFillColor(GBL::COLOR::TEXT);
	version.setFillColor(GBL::COLOR::TEXT);

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;

	if (!downloadButtonTexture.loadFromFile(GBL::DIR::textures + "get_app_1x.png"))
		downloadButton.setFillColor(sf::Color(sf::Color::Green));
	else
		downloadButton.setFillColor(GBL::COLOR::ITEM::ICON);
	downloadButtonTexture.setSmooth(true);
	downloadButton.setTexture(&downloadButtonTexture);
	downloadButton.setSize(sf::Vector2f(24, 24));
	downloadButton.setOrigin(sf::Vector2f(downloadButton.getLocalBounds().width / 2, downloadButton.getLocalBounds().height / 2));
	downloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	if (redownloadButtonTexture.loadFromFile(GBL::DIR::textures + "auto_renew_1x.png"))
		redownloadButton.setFillColor(GBL::COLOR::ITEM::ICON);
	else
		redownloadButton.setFillColor(sf::Color::Yellow);
	redownloadButtonTexture.setSmooth(true);
	redownloadButton.setTexture(&redownloadButtonTexture);
	redownloadButton.setRadius(10);
	redownloadButton.setRotation(30);
	redownloadButton.setOrigin(sf::Vector2f(redownloadButton.getLocalBounds().width / 2, redownloadButton.getLocalBounds().height / 2));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y - 15));

	if (!removeButtonTexture.loadFromFile(GBL::DIR::textures + "delete_forever_1x.png"))
		removeButton.setFillColor(sf::Color::Red);
	else
		removeButton.setFillColor(GBL::COLOR::ITEM::ICON);
	removeButtonTexture.setSmooth(true);
	removeButton.setTexture(&removeButtonTexture);
	removeButton.setSize(sf::Vector2f(24, 24));
	removeButton.setOrigin(sf::Vector2f(removeButton.getLocalBounds().width / 2, removeButton.getLocalBounds().height / 2));
	removeButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 15));

	if (!launchButtonTexture.loadFromFile(GBL::DIR::textures + "launch_1x.png"))
		launchButton.setFillColor(sf::Color::Green);
	else
		launchButton.setFillColor(GBL::COLOR::ITEM::ICON);
	launchButtonTexture.setSmooth(true);
	launchButton.setTexture(&launchButtonTexture);
	launchButton.setSize(sf::Vector2f(20, 20));
	launchButton.setOrigin(sf::Vector2f(launchButton.getLocalBounds().width / 2, launchButton.getLocalBounds().height / 2));
	launchButton.setPosition(sf::Vector2f(fuckedUpXPosition - 28, cardShape.getPosition().y));

	std::cout << "card is ready (took " << itemCreateTimer.getElapsedTime().asSeconds() << " seconds)" << std::endl;
}

StoreApp::~StoreApp()
{
	delete targetWindow;
}


void StoreApp::deleteFiles()
{
	std::cout << "using deprecated file deletion method" << std::endl;

	try
	{
		fs::remove(itemInstallDir + "release.zip");
		std::cout << "done" << std::endl;
		downloaded = false;
	}
	catch (const std::exception& e)
	{
		std::cerr << "\n" << "failed to delete file" << std::endl;
		std::cerr << "\n" << e.what() << std::endl;
	}
}

bool StoreApp::checkForUpdate()
{
	//TODO: give apps a variable in info.dat called release
	//compare release to current, as the launcher updater does

	std::cout << "checking for updates" << std::endl;

	Download getRemoteVersion;
	getRemoteVersion.setInput(".//" + GBL::WEB::APPS + info.name + "//info.dat");
	getRemoteVersion.download();

	std::string rVersion, lVersion = version.getString();

	SettingsParser getVersion;
	if (getVersion.loadFromFile(".//" + GBL::WEB::APPS + info.name + "//info.dat"))
		getVersion.get("version", rVersion);
	else
	{
		rVersion = lVersion;
		std::cerr << "failed to get remote app version" << std::endl;
	}

	if (lVersion != rVersion)
	{
		std::cout << "item is out of date! (local: " << lVersion << " : remote: " << rVersion << ")" << std::endl;

		updateIsAvailable = true;
		redownloadButton.setFillColor(sf::Color::Yellow);

		version.setString(lVersion + " (New " + rVersion + "!)");

		return true;
	}
	else
	{
		std::cout << "item is up to date! :D (local: " << lVersion << " : remote: " << rVersion << ")" << std::endl;

		return false;
	}
}

void StoreApp::updateItem()
{
	std::cout << "updating item" << std::endl;

	download();
}

void StoreApp::download()
{
	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));
	isDownloading = true;

	if (fs::exists(itemInstallDir + "/release.zip"))
	{
		std::cout << "updating " << info.name << std::endl;

		downloadIcon();
		downloadInfo();
		downloadFiles();

		std::cout << "\n" << "finished updating " << info.name << std::endl;
	}
	else
	{
		std::cout << "downloading " << info.name << std::endl;

		downloadInfo();

		downloadIcon();
		iconTexture.loadFromFile(itemInstallDir + "icon.png");

		downloadFiles();

		std::cout << "\n" << "downloading update " << info.name << std::endl;
	}

	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y - 15));

	if (updateIsAvailable)
	{
		redownloadButton.setFillColor(sf::Color::White);
		updateIsAvailable = false;
	}

	redownloadButton.setRotation(30);
	isDownloading = false;
}

void StoreApp::openItem()
{
	//TODO: proper system for opening and monitoring apps

#ifdef _WIN32
	std::cout << "opening item" << std::endl;
	std::string launch = "start " + itemInstallDir + "release.zip -kunlaunched";
	system(launch.c_str());
#else
	std::cerr << "Opening stuff is not supported on this system." << std::endl;

	MessageBox::Options modOptions;
	modOptions.title = "Unsupported Platform";
	modOptions.text = "Opening apps is not supported on your platform.\n\nTo open the app:\nInstallation Dir -> bin -> apps -> appname";
	modOptions.settings = { "Sad" };

	MessageBox platformAlert(modOptions);
	platformAlert.runBlocking();
#endif
}

void StoreApp::updateSize(float xSize, float ySize, float xPos, float yPos)
{
	cardShape.setSize(sf::Vector2f(xSize, 75));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f(xPos, cardShape.getPosition().y)); // probably not the best
	totalHeight = cardShape.getLocalBounds().height;

	icon.setOrigin(sf::Vector2f(icon.getLocalBounds().width / 2, icon.getLocalBounds().height / 2));
	icon.setPosition(sf::Vector2f(cardShape.getPosition().x - (cardShape.getSize().x / 2) + icon.getLocalBounds().width / 2, cardShape.getPosition().y));

	name.setPosition(static_cast<int>(icon.getPosition().x + 45), static_cast<int>(cardShape.getPosition().y - 40));
	description.setPosition(static_cast<int>(icon.getPosition().x + 45), static_cast<int>(cardShape.getPosition().y - 15));
	version.setPosition(static_cast<int>(icon.getPosition().x + 45), static_cast<int>(cardShape.getPosition().y + 10));

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;

	downloadButton.setOrigin(sf::Vector2f(downloadButton.getLocalBounds().width / 2, downloadButton.getLocalBounds().height / 2));
	downloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	redownloadButton.setRotation(30);
	redownloadButton.setOrigin(sf::Vector2f(redownloadButton.getLocalBounds().width / 2, redownloadButton.getLocalBounds().height / 2));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y - 15));

	removeButton.setOrigin(sf::Vector2f(removeButton.getLocalBounds().width / 2, removeButton.getLocalBounds().height / 2));
	removeButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 15));

	launchButton.setOrigin(sf::Vector2f(launchButton.getLocalBounds().width / 2, launchButton.getLocalBounds().height / 2));
	launchButton.setPosition(sf::Vector2f(fuckedUpXPosition - 28, cardShape.getPosition().y));
}

void StoreApp::draw()
{
	targetWindow->draw(cardShape);
	//	targetWindow->draw(controlBar);
	targetWindow->draw(icon);

	targetWindow->draw(name);
	targetWindow->draw(description);
	targetWindow->draw(version);

	if (isDownloading)
	{
		redownloadButton.rotate(1);
		targetWindow->draw(redownloadButton);
	}
	else // not downloading
	{
		if (downloaded)
		{
			targetWindow->draw(redownloadButton);
			targetWindow->draw(removeButton);
			targetWindow->draw(launchButton);
		}
		else
		{
			if (!missing)
				targetWindow->draw(downloadButton);
		}
	}
}

// private

std::ifstream& StoreApp::GotoLine(std::ifstream& file, unsigned int line)
{
	file.seekg(std::ios::beg); // go to top

	for (size_t i = 0; i < line - 1; ++i) // go to line
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return file; // return line
}

void StoreApp::parseInfo(std::string dir) // a lot easier than I thought it would be.
{
	//TODO: add checks here to make sure we don't try to parse a file that ended up being a 500 or 404

	std::cout << "parsing info for " << dir << std::endl;

	if (fs::exists(dir + "info.dat"))
	{
		std::string name_;
		std::string description_;
		std::string version_;

		SettingsParser itemInfo;
		if (itemInfo.loadFromFile(dir + "info.dat"))
		{
			if (itemInfo.get("name", info.name))
				name.setString(info.name);
			else
			{
				name.setStyle(sf::Text::Style::Italic);
				name.setString('"' + info.name + '"');
			}

			if (itemInfo.get("description", info.description))
				description.setString(info.description);
			else
			{
				description.setStyle(sf::Text::Style::Italic);
				description.setString("No description provided.");
			}

			if (itemInfo.get("version", version_))
				version.setString(version_);
			else
			{
				version.setStyle(sf::Text::Style::Italic);
				version.setString("Unknown");
			}
		}
		else
		{
			name.setString("Unable to load item info!");
			description.setString("Try downloading the app.");
			version.setString("");
		}
	}
	else
	{
		std::cout << "info file is empty or missing" << std::endl;

		iconTexture.loadFromFile(GBL::DIR::textures + "error_2x.png");
		updateIsAvailable = true; // because there's no way to represent an error yet, we just mark it for needing to be redownloaded

		name.setString("missing info for \"" + info.name + "\"");
		description.setString("missing info.dat; try redownloading");

		missing = true;
	}
}

int StoreApp::downloadIcon()
{
	std::cout << "\n" << "downloading icon" << std::endl;

	Download getIcon;
	getIcon.setInput(".//" + GBL::WEB::APPS + info.name + "//icon.png");
	getIcon.setOutputDir(GBL::DIR::apps + info.name + "//");
	getIcon.setOutputFilename("icon.png");
	getIcon.download();
	getIcon.save();

	iconTexture.loadFromFile(itemInstallDir + "icon.png");

	return 1;
}

int StoreApp::downloadInfo()
{
	std::cout << "\n" << "downloading info" << std::endl;

	Download getInfo;
	getInfo.setInput(".//" + GBL::WEB::APPS + info.name + "//info.dat");
	getInfo.setOutputDir(GBL::DIR::apps + info.name + "//");
	getInfo.setOutputFilename("info.dat");
	getInfo.download();
	getInfo.save();

	parseInfo(itemInstallDir);

	return 1;
}

int StoreApp::downloadFiles()
{
	std::cout << "\n" << "downloading files" << std::endl;

	Download getFiles;
	getFiles.setInput(".//" + GBL::WEB::APPS + info.name + "//release.zip");
	getFiles.setOutputDir(GBL::DIR::apps + info.name + "//");
	getFiles.setOutputFilename("release.zip");
	getFiles.download();
	getFiles.save();

	downloaded = true;

	return 1;
}

int StoreApp::downloadFile(std::string fileName, std::string inPath, std::string outPath)
{
	std::cout << "deleting \"" + fileName + "\"" << std::endl;

	std::cout << "\n" << "downloading \"" + fileName + "\"" << std::endl;

	Download getInfo;
	getInfo.setInput(inPath);
	getInfo.setOutputDir(outPath);
	getInfo.setOutputFilename(fileName);
	getInfo.download();
	getInfo.save();

	if (fs::exists(outPath + "//" + fileName))
	{
		std::cout << "success" << std::endl;

		return 1;
	}
	else
	{
		std::cerr << "failed to download file" << std::endl;

		return 0;
	}
}

int StoreApp::deleteFile(std::string fileName, std::string filePath)
{
	std::cout << "deleting \"" + fileName + "\"" << std::endl;

	try
	{
		fs::remove(filePath + "//" + fileName);
		std::cout << "verifying" << std::endl;

		if (fs::exists(filePath + "//" = fileName))
		{
			std::cerr << "failed to remove file" << std::endl;

			return 0;
		}
		else
		{
			std::cout << "success" << std::endl;

			return 1;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "failed to remove file: " << std::endl;
		std::cerr << e.what() << std::endl;

		return 0;
	}
}
