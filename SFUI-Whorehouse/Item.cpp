#include "Item.hpp"
#include "Download.hpp"
#include "Globals.hpp"
#include "MessageBox.hpp"
#include "SettingsParser.hpp"

#include <fstream>
#include <iostream>
#include <experimental\filesystem>
#include <SFML\Network.hpp>

namespace fs = std::experimental::filesystem;

Item::Item(std::string itemName_, sf::RenderWindow* target_window, float xSize, float ySize, float xPos, float yPos)
{
	sf::Clock itemCreateTimer;

	std::cout << "creating new card for \"" + itemName_ + "\"" << std::endl;

	targetWindow = target_window;
	itemName = itemName_;
	installDir = ".\\" + GBL::DIR::BASE + GBL::DIR::APPS + itemName + "\\"; // .\\bin\\apps\\itemName\\

	if (!fs::exists(installDir))
	{
		std::cout << "app directory does not already exist, creating..." << std::endl;

		try
		{
			fs::create_directory(installDir);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;

			std::cout << "failed to create directory" << std::endl;
		}
	}

	if (fs::exists(installDir + "info.dat"))
	{
		std::cout << "info was found, parsing" << std::endl;

		parseInfo(installDir);
	}
	else // info is not downloaded
	{
		std::cout << "info was not found, downloading" << std::endl;

		downloadInfo();
	}

	if (fs::exists(installDir + "icon.png"))
	{
		std::cout << "icon was found" << std::endl;

		iconTexture.loadFromFile(installDir + "icon.png");
	}
	else // icon is not downloaded
	{
		std::cout << "icon was not found, downloading" << std::endl;

		downloadIcon();
	}

	if (fs::exists(installDir + "release.zip"))
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

	font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf");

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

	if (!downloadButtonTexture.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "get_app_1x.png"))
		downloadButton.setFillColor(sf::Color(sf::Color::Green));
	else
		downloadButton.setFillColor(GBL::COLOR::ITEM::ICON);
	downloadButtonTexture.setSmooth(true);
	downloadButton.setTexture(&downloadButtonTexture);
	downloadButton.setSize(sf::Vector2f(24, 24));
	downloadButton.setOrigin(sf::Vector2f(downloadButton.getLocalBounds().width / 2, downloadButton.getLocalBounds().height / 2));
	downloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	if (redownloadButtonTexture.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "auto_renew_1x.png"))
		redownloadButton.setFillColor(GBL::COLOR::ITEM::ICON);
	else
		redownloadButton.setFillColor(sf::Color::Yellow);
	redownloadButtonTexture.setSmooth(true);
	redownloadButton.setTexture(&redownloadButtonTexture);
	redownloadButton.setRadius(10);
	redownloadButton.setRotation(30);
	redownloadButton.setOrigin(sf::Vector2f(redownloadButton.getLocalBounds().width / 2, redownloadButton.getLocalBounds().height / 2));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y - 15));

	if (!removeButtonTexture.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "delete_forever_1x.png"))
		removeButton.setFillColor(sf::Color::Red);
	else
		removeButton.setFillColor(GBL::COLOR::ITEM::ICON);
	removeButtonTexture.setSmooth(true);
	removeButton.setTexture(&removeButtonTexture);
	removeButton.setSize(sf::Vector2f(24, 24));
	removeButton.setOrigin(sf::Vector2f(removeButton.getLocalBounds().width / 2, removeButton.getLocalBounds().height / 2));
	removeButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 15));

	if (!launchButtonTexture.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "launch_1x.png"))
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

Item::~Item()
{
	delete targetWindow;
}

// public:

void Item::deleteFiles()
{
	std::cout << "using deprecated file deletion method" << std::endl;

	try
	{
		fs::remove(installDir + "release.zip");
		std::cout << "done" << std::endl;
		downloaded = false;
	}
	catch (const std::exception& e)
	{
		std::cout << "\n" << e.what() << std::endl;
	}
}

bool Item::checkForUpdate()
{
	std::cout << "checking for updates" << std::endl;

	Download2 getRemoteVersion;
	getRemoteVersion.setInput(".\\" + GBL::DIR::WEB_APP_DIRECTORY + itemName + "\\info.dat");
	getRemoteVersion.download();

	getRemoteVersion.fileBuffer.erase(0, getRemoteVersion.fileBuffer.find('\n') + 1);
	getRemoteVersion.fileBuffer.erase(0, getRemoteVersion.fileBuffer.find('\n') + 1);
	getRemoteVersion.fileBuffer.erase(getRemoteVersion.fileBuffer.find('\n'), getRemoteVersion.fileBuffer.length());

	getRemoteVersion.fileBuffer.erase(0, getRemoteVersion.fileBuffer.find_first_of('"') + 1);
	getRemoteVersion.fileBuffer.erase(getRemoteVersion.fileBuffer.find_last_of('"'), getRemoteVersion.fileBuffer.length());
	std::string rVersion = getRemoteVersion.fileBuffer;
	std::string lVersion = version.getString();

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

void Item::updateItem()
{
	std::cout << "updating item" << std::endl;

	download();
}

void Item::download()
{
	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));
	isDownloading = true;

	if (fs::exists(installDir + "/release.zip"))
	{
		std::cout << "updating " << itemName << std::endl;

		downloadIcon();
		downloadInfo();
		downloadFiles();

		std::cout << "\n" << "finished updating " << itemName << std::endl;
	}
	else
	{
		std::cout << "downloading " << itemName << std::endl;

		downloadInfo();

		downloadIcon();
		iconTexture.loadFromFile(installDir + "icon.png");

		downloadFiles();

		std::cout << "\n" << "downloading update " << itemName << std::endl;
	}

	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y - 15));

	if (updateIsAvailable)
	{
		redownloadButton.setFillColor(sf::Color::White);
		updateIsAvailable = false;
	}

	redownloadButton.setRotation(30);
	isDownloading = false;

	parseInfo(installDir);
}

void Item::openItem()
{
#ifdef _WIN32
	std::cout << "opening item" << std::endl;
	std::string launch = "start " + installDir + "release.zip";
	system(launch.c_str());
#else
	std::cout << "Your system does not support this function!" << std::endl;

	MessageBoxOptions modOptions;
	modOptions.title = "Unsupported Platform";
	modOptions.text = "I can't open the thing because you're probably not using Windows. How sad!";
	modOptions.settings = { "Ok" };

	MessageBox platformAlert(modOptions);
	platformAlert.runBlocking();
#endif
}

void Item::updateSize(float xSize, float ySize, float xPos, float yPos)
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

void Item::draw()
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
			targetWindow->draw(downloadButton);
		}
	}
}

// privatez

std::ifstream& Item::GotoLine(std::ifstream& file, unsigned int line)
{
	file.seekg(std::ios::beg); // go to top

	for (size_t i = 0; i < line - 1; ++i) // go to line
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return file; // return line
}

void Item::parseInfo(std::string dir) // a lot easier than I thought it would be.
{
	//TODO: add checks here to make sure we don't try to parse a file that ended up being a 500 or 404

	std::cout << "parsing info for " << dir << std::endl;

	if (fs::exists(dir + "info.dat") && (fs::file_size(dir + "info.dat") != 0))
	{
		std::ifstream getit(dir + "info.dat", std::ios::in);

		std::string name_;
		std::string description_;
		std::string version_;

		/*
		// line 1, the name
		getline(getit, name_);
		// line 2, description
		getline(getit, description_);
		// line 3, version
		getline(getit, version_);

		name_.erase(0, name_.find_first_of('"') + 1);
		name_.erase(name_.find_last_of('"'), name_.length());
		name.setString(name_);

		description_.erase(0, description_.find_first_of('"') + 1);
		description_.erase(description_.find_last_of('"'), description_.length());
		description.setString(description_);

		version_.erase(0, version_.find_first_of('"') + 1);
		version_.erase(version_.find_last_of('"'), version_.length());
		version.setString(version_);
		*/

		SettingsParser itemInfo;
		if (itemInfo.loadFromFile(dir + "info.dat"))
		{
			if (itemInfo.get("name", name_))
				name.setString(name_);

			if (itemInfo.get("description", description_))
				description.setString(description_);

			if (itemInfo.get("version", version_))
				version.setString(version_);
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

		iconTexture.loadFromFile(".\\" + GBL::DIR::BASE + GBL::DIR::RESOURCE + GBL::DIR::TEXTURE + "error_1x.png");
		updateIsAvailable = true; // because there's no way to represent an error yet, we just mark it for needing to be redownloaded

		name.setString("INFO IS MISSING");
		description.setString("this app's info.dat is missing, try rewdownloading.");
	}
}

int Item::downloadIcon()
{
	std::cout << "\n" << "downloading icon" << std::endl;

	Download2 getIcon;
	getIcon.setInput(".\\" + GBL::DIR::WEB_APP_DIRECTORY + itemName + "\\icon.png");
	getIcon.setOutputDir(".\\" + GBL::DIR::BASE + GBL::DIR::APPS + itemName + "\\");
	getIcon.setOutputFilename("icon.png");
	getIcon.download();
	getIcon.save();

	iconTexture.loadFromFile(installDir + "icon.png");

	return 1;
}

int Item::downloadInfo()
{
	std::cout << "\n" << "downloading info" << std::endl;

	Download2 getInfo;
	getInfo.setInput(".\\" + GBL::DIR::WEB_APP_DIRECTORY + itemName + "\\info.dat");
	getInfo.setOutputDir(".\\" + GBL::DIR::BASE + GBL::DIR::APPS + itemName + "\\");
	getInfo.setOutputFilename("info.dat");
	getInfo.download();
	getInfo.save();

	parseInfo(installDir);

	return 1;
}

int Item::downloadFiles()
{
	std::cout << "\n" << "downloading files" << std::endl;

	Download2 getFiles;
	getFiles.setInput(".\\" + GBL::DIR::WEB_APP_DIRECTORY + itemName + "\\release.zip");
	getFiles.setOutputDir(".\\" + GBL::DIR::BASE + GBL::DIR::APPS + itemName + "\\");
	getFiles.setOutputFilename("release.zip");
	getFiles.download();
	getFiles.save();

	downloaded = true;

	return 1;
}

int Item::downloadFile(std::string fileName, std::string inPath, std::string outPath)
{
	std::cout << "deleting \"" + fileName + "\"" << std::endl;

	std::cout << "\n" << "downloading \"" + fileName + "\"" << std::endl;

	Download2 getInfo;
	getInfo.setInput(inPath);
	getInfo.setOutputDir(outPath);
	getInfo.setOutputFilename(fileName);
	getInfo.download();
	getInfo.save();

	if (fs::exists(outPath + "\\" + fileName))
	{
		std::cout << "success" << std::endl;

		return 1;
	}
	else
	{
		std::cout << "failed to download file" << std::endl;

		return 0;
	}
}

int Item::deleteFile(std::string fileName, std::string filePath)
{
	std::cout << "deleting \"" + fileName + "\"" << std::endl;

	try
	{
		fs::remove(filePath + "\\" + fileName);
		std::cout << "verifying" << std::endl;

		if (fs::exists(filePath + "\\" = fileName))
		{
			std::cout << "failed to remove file" << std::endl;

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
		std::cout << "\n" << e.what() << std::endl;

		return 0;
	}
}
