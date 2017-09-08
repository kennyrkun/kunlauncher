#include <fstream>
#include <iostream>
#include <experimental\filesystem>
#include <SFML\Network.hpp>

#include "Item.hpp"
#include "Download.hpp"
#include "Globals.hpp"

namespace fs = std::experimental::filesystem;

Item::Item(std::string itemName_, sf::RenderWindow* target_window, float yPos)
{
	std::cout << "-----------------------------" << std::endl;
	std::cout << "creating new card for \"" + itemName_ + "\"" << std::endl;

	targetWindow = target_window;
	itemName = itemName_;
	installDir = ".\\" + CONST::DIR::BASE + "\\apps\\" + itemName + "\\"; /// .\\bin\\apps\\itemName\\

	if (!fs::exists(installDir))
	{
		std::cout << "app directory does not already exist, creating...";

		try
		{
			fs::create_directory(installDir);

			std::cout << " done" << std::endl;
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

		Download downloadInfo;
		downloadInfo.setInputPath(itemName + "/info.dat");
		downloadInfo.setOutputDir(installDir);
		downloadInfo.setOutputFilename("info.dat");

		switch (downloadInfo.download())
		{
		case sf::Http::Response::Status::Ok:
			std::cout << "saving info" << std::endl;

			downloadInfo.save();
			parseInfo(installDir);
			break;

		case sf::Http::Response::Status::InternalServerError:
			std::cout << "failed to download info, aborting" << std::endl;
			name.setString("Failed to load app!");
			description.setString("Encountered 500 Internal Server Error during download");
			break;

		default:
			break;
		}
	}

	if (fs::exists(installDir + "icon.png"))
	{
		std::cout << "icon was found" << std::endl;

		iconTexture.loadFromFile(installDir + "icon.png");
	}
	else // icon is not downloaded
	{
		std::cout << "icon was not found, downloading" << std::endl;

		Download downloadIcon;
		downloadIcon.setInputPath(itemName + "/icon.png");
		downloadIcon.setOutputDir(installDir);
		downloadIcon.setOutputFilename("icon.png");

		switch (downloadIcon.download())
		{
		case sf::Http::Response::Status::Ok:
			std::cout << "saving icon" << std::endl;

			downloadIcon.save();
			iconTexture.loadFromFile(installDir + "icon.png");
			break;

		case sf::Http::Response::Status::InternalServerError:
			std::cout << "failed to download icon, aborting" << std::endl;
			name.setString("Failed to download icon");
			description.setString("Encountered 500 Internal Server Error during download");
			break;

		default:
			break;
		}
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

	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 75));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5.0f, yPos)); // probably not the best
	cardShape.setFillColor(CONST::COLOR::ITEM::CARD);

	totalHeight = cardShape.getLocalBounds().height + 10;

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

	name.setFillColor(sf::Color::White);
	description.setFillColor(sf::Color::White);
	version.setFillColor(sf::Color::White);

	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;

	if (!downloadButtonTexture.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::TEXTURE + "\\get_app_1x.png"))
		downloadButton.setFillColor(sf::Color::Green);
	downloadButtonTexture.setSmooth(true);
	downloadButton.setTexture(&downloadButtonTexture);
	downloadButton.setSize(sf::Vector2f(24, 24));
	downloadButton.setOrigin(sf::Vector2f(downloadButton.getLocalBounds().width / 2, downloadButton.getLocalBounds().height / 2));
	downloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	redownloadButtonTexture.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::TEXTURE + "\\auto_renew_1x.png");
	redownloadButtonTexture.setSmooth(true);
	redownloadButton.setTexture(&redownloadButtonTexture);
	redownloadButton.setRadius(10);
	redownloadButton.setRotation(30);
	redownloadButton.setOrigin(sf::Vector2f(redownloadButton.getLocalBounds().width / 2, redownloadButton.getLocalBounds().height / 2));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y - 15));

	if (!removeButtonTexture.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::TEXTURE + "\\delete_forever_1x.png"))
		removeButton.setFillColor(sf::Color::Red);
	removeButtonTexture.setSmooth(true);
	removeButton.setTexture(&removeButtonTexture);
	removeButton.setSize(sf::Vector2f(24, 24));
	removeButton.setOrigin(sf::Vector2f(removeButton.getLocalBounds().width / 2, removeButton.getLocalBounds().height / 2));
	removeButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 15));

	if (!launchButtonTexture.loadFromFile(".\\" + CONST::DIR::BASE + "\\" + CONST::DIR::RESOURCE + "\\" + CONST::DIR::TEXTURE + "\\launch_1x.png"))
		launchButton.setFillColor(sf::Color::Green);
	launchButtonTexture.setSmooth(true);
	launchButton.setTexture(&launchButtonTexture);
	launchButton.setSize(sf::Vector2f(20, 20));
	launchButton.setOrigin(sf::Vector2f(launchButton.getLocalBounds().width / 2, launchButton.getLocalBounds().height / 2));
	launchButton.setPosition(sf::Vector2f(fuckedUpXPosition - 28, cardShape.getPosition().y));

	std::cout << "card is ready" << std::endl;
	std::cout << "-----------------------------" << std::endl;
}

Item::~Item()
{
	delete targetWindow;
}

// public:

void Item::deleteFiles()
{
	std::cout << "using deprecated removal method" << std::endl;

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

	Download getNewVersion;
	getNewVersion.setInputPath(itemName + "/info.dat");
	getNewVersion.download();

	getNewVersion.fileBuffer.erase(0, getNewVersion.fileBuffer.find('\n') + 1);
	getNewVersion.fileBuffer.erase(0, getNewVersion.fileBuffer.find('\n') + 1);
	getNewVersion.fileBuffer.erase(getNewVersion.fileBuffer.find('\n'), getNewVersion.fileBuffer.length());

	getNewVersion.fileBuffer.erase(0, getNewVersion.fileBuffer.find_first_of('"') + 1);
	getNewVersion.fileBuffer.erase(getNewVersion.fileBuffer.find_last_of('"'), getNewVersion.fileBuffer.length());
	std::string rVersion = getNewVersion.fileBuffer;
	std::string lVersion = version.getString();

	if (lVersion != rVersion)
	{
		std::cout << "item is out of date! (local: " << lVersion << " : remote: " << rVersion << ")" << std::endl;

		updateIsAvailable = true;
		redownloadButton.setFillColor(sf::Color::Yellow);

		version.setString(lVersion + " (New " + rVersion + "!)");
	}
	else
	{
		std::cout << "item is up to date! :D (local: " << lVersion << " : remote: " << rVersion << ")" << std::endl;
	}

	return false;
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
		icon.setTexture(&iconTexture, true);

		downloadFiles();

		std::cout << "\n" << "downloading update" << itemName << std::endl;
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
	std::cout << "opening item" << std::endl;

#if defined (_WIN32)
	std::string launch = "start " + installDir + "release.zip";
	system((launch).c_str());
#endif
}

void Item::update(float yPos)
{
	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 75));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5.0f, yPos)); // probably not the best

	totalHeight = cardShape.getLocalBounds().height + 10;

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
			//		button.draw(*targetWindow);
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
	//TODO: add checks here to make sure we don't try to parse a file that ended up being a 500

	std::ifstream getit(dir + "info.dat", std::ios::in);

	std::string name_;
	std::string description_;
	std::string version_;

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
}

int Item::downloadIcon()
{
	std::cout << "\n" << "downloading icon" << std::endl;

	Download getIcon;
	getIcon.setInputPath(itemName + "/icon.png");
	getIcon.setOutputDir(installDir);
	getIcon.setOutputFilename("icon.png");

	if (getIcon.download() == sf::Http::Response::Status::Ok)
	{
		getIcon.save();
	}
	else
	{
		std::cout << "failed to download icon <???>" << std::endl;
	}

	return 1;
}

int Item::downloadInfo()
{
	std::cout << "\n" << "downloading info" << std::endl;

	Download getInfo;
	getInfo.setInputPath(itemName + "/info.dat");
	getInfo.setOutputDir(installDir);
	getInfo.setOutputFilename("info.dat");
	getInfo.download();
	getInfo.save();

	return 1;
}

int Item::downloadFiles()
{
	std::cout << "\n" << "downloading files" << std::endl;

	Download getInfo;
	getInfo.setInputPath(itemName + "/release.zip");
	getInfo.setOutputDir(installDir);
	getInfo.setOutputFilename("release.zip");
	getInfo.download();
	getInfo.save();

	downloaded = true;

	return 1;
}

int Item::downloadFile(std::string fileName, std::string inPath, std::string outPath)
{
	std::cout << "deleting \"" + fileName + "\"" << std::endl;

	std::cout << "\n" << "downloading \"" + fileName + "\"" << std::endl;

	Download getInfo;
	getInfo.setInputPath(inPath);
	getInfo.setOutputDir(outPath);
	getInfo.setOutputFilename(fileName);

	if (getInfo.download() == sf::Http::Response::Status::Ok)
	{
		getInfo.save();
	}
	else
	{
		std::cout << "failed to download \"" + itemName + "\"" << std::endl;

		return 0;
	}

	std::cout << "verifying" << std::endl;

	if (fs::exists(outPath + "\\" + fileName))
	{
		std::cout << "success" << std::endl;

		return 1;
	}
	else
	{
		std::cout << "failed to remove file" << std::endl;

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
