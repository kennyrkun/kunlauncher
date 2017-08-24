#include <fstream>
#include <iostream>
#include <experimental\filesystem>

#include "Item.hpp"
#include "Download.hpp"
#include "constants.hpp"

namespace fs = std::experimental::filesystem;

Item::Item(std::string itemName_, sf::RenderWindow* target_window, float yPos)
{
	std::cout << "creating new card for \"" + itemName_ + "\"" << std::endl;

	targetWindow = target_window;
	itemName = itemName_;

	installDir = ".\\" + BASE_DIRECTORY + "\\apps\\" + itemName + "\\";

	cardShape.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 75));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f((targetWindow->getSize().x / 2) - 5.0f, yPos)); // probably not the best
	cardShape.setFillColor(sf::Color(100, 100, 100));

	// (left/right, up/down)
//	controlBar.setSize(sf::Vector2f(targetWindow->getSize().x - 25, 25));
//	controlBar.setOrigin(sf::Vector2f(controlBar.getLocalBounds().width / 2, controlBar.getLocalBounds().height / 2));
//	controlBar.setPosition(sf::Vector2f(cardShape.getPosition().x, cardShape.getPosition().y + cardShape.getLocalBounds().height / 2 + controlBar.getLocalBounds().height / 2 + 2.0f /* padding */)); // probably not the best
//	controlBar.setFillColor(sf::Color(100, 100, 100));

//	totalHeight = cardShape.getLocalBounds().height + controlBar.getLocalBounds().height + 10;
	totalHeight = cardShape.getLocalBounds().height + 10.0f;

	icon.setSize(sf::Vector2f(cardShape.getSize().y, cardShape.getSize().y)); // a square
	icon.setOrigin(sf::Vector2f(icon.getLocalBounds().width / 2, icon.getLocalBounds().height / 2));
	icon.setPosition(sf::Vector2f(cardShape.getPosition().x - (cardShape.getSize().x / 2) + icon.getLocalBounds().width / 2, cardShape.getPosition().y));
	iconTexture.setSmooth(true);

//	button.setPosition(sf::Vector2f(controlBar.getPosition().x + (controlBar.getLocalBounds().height), controlBar.getPosition().y));

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

	if (!downloadButtonTexture.loadFromFile(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + TEXTURE_DIRECTORY + "\\get_app_1x.png"))
		downloadButton.setFillColor(sf::Color::Green);
	downloadButtonTexture.setSmooth(true);
	downloadButton.setTexture(&downloadButtonTexture);
	downloadButton.setSize(sf::Vector2f(24, 24));
	downloadButton.setOrigin(sf::Vector2f(downloadButton.getLocalBounds().width / 2, downloadButton.getLocalBounds().height / 2));
	downloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));

	redownloadButtonTexture.loadFromFile(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + TEXTURE_DIRECTORY + "\\auto_renew_1x.png");
	redownloadButtonTexture.setSmooth(true);
	redownloadButton.setTexture(&redownloadButtonTexture);
	redownloadButton.setRadius(10);
	redownloadButton.setRotation(30);
	redownloadButton.setOrigin(sf::Vector2f(redownloadButton.getLocalBounds().width / 2, redownloadButton.getLocalBounds().height / 2));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y - 15));

	if (!removeButtonTexture.loadFromFile(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + TEXTURE_DIRECTORY + "\\delete_forever_1x.png"))
		removeButton.setFillColor(sf::Color::Red);
	removeButtonTexture.setSmooth(true);
	removeButton.setTexture(&removeButtonTexture);
	removeButton.setSize(sf::Vector2f(24, 24));
	removeButton.setOrigin(sf::Vector2f(removeButton.getLocalBounds().width / 2, removeButton.getLocalBounds().height / 2));
	removeButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 15));

	if (!launchButtonTexture.loadFromFile(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + TEXTURE_DIRECTORY + "\\launch_1x.png"))
		launchButton.setFillColor(sf::Color::Green);
	launchButtonTexture.setSmooth(true);
	launchButton.setTexture(&launchButtonTexture);
	launchButton.setSize(sf::Vector2f(20, 20));
	launchButton.setOrigin(sf::Vector2f(launchButton.getLocalBounds().width / 2, launchButton.getLocalBounds().height / 2));
	launchButton.setPosition(sf::Vector2f(fuckedUpXPosition - 28, cardShape.getPosition().y));

	if (fs::exists(installDir)) // if we know about it
	{
		if (!fs::exists(installDir + "icon.png")) // if it's missing the icon
		{
			std::cout << "icon is missing, downloading..." << std::endl;

			downloadIcon();
		}

		if (!fs::exists(installDir + "info.dat")) // if it's missing the info
		{
			std::cout << "info is missing, downloading..." << std::endl;

			downloadInfo();
			parseInfo(installDir);
		}
		else
		{
			parseInfo(installDir);
		}

		if (fs::exists(installDir + "release.zip")) // if it's installed
		{
			cardShape.setFillColor(sf::Color(100, 100, 100));
			downloaded = true;
			checkForUpdate();
		}
	}
	else // if we don't know about it, learn about it
	{
		downloadIcon();
		downloadInfo();
		parseInfo(installDir); // teach us what we learned.
	}

	std::cout << "card is ready" << std::endl;
}

Item::~Item()
{
	delete targetWindow;
}

// public:

void Item::deleteFiles()
{
	std::cout << "deleting files... ";

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

	Download getNewVersion(true);
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
	}
	else
	{
		std::cout << "item is up to date! :D (local: " << lVersion << " : remote: " << rVersion << ")" << std::endl;
	}

	return false;
}

void Item::updateItem()
{
	download();
}

void Item::download()
{
	float fuckedUpXPosition = (cardShape.getPosition().x + (cardShape.getLocalBounds().width / 2)) - 30;
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y));
	isDownloading = true;

	deleteIcon();
	deleteInfo();

	if (fs::exists(installDir + "/release.zip"))
	{
		std::cout << "updating " << itemName << std::endl;

		deleteFiles();

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

		std::cout << "\n" << "downloading updating " << itemName << std::endl;
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
#if defined (_WIN32)
	std::string launch = "start " + installDir + "release.zip";
	system((launch).c_str());
#endif
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

// private

std::ifstream& Item::GotoLine(std::ifstream& file, unsigned int num)
{
	file.seekg(std::ios::beg); // go to top

	for (size_t i = 0; i < num - 1; ++i) // go to line
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return file; // return line
}

void Item::parseInfo(std::string dir) // a lot easier than I thought it would be.
{
	if (!iconTexture.loadFromFile(installDir + "icon.png"))
	{
		std::cout << "could not load icon texture" << std::endl;
		icon.setFillColor(sf::Color::Magenta);
	}
	else
	{
		icon.setTexture(&iconTexture);
	}

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
	// line 4, the filename
	getline(getit, fileName); // reads directly into the file. we don't use this (yet) so we don't need to modify it.

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

void Item::downloadIcon()
{
	std::cout << "\n" << "downloading icon" << std::endl;

	Download getIcon;
	getIcon.setInputPath(itemName + "/icon.png");
	getIcon.setOutputDir(installDir);
	getIcon.setOutputFile("icon.png");
	getIcon.download();
	getIcon.save();
}

void Item::downloadInfo()
{
	std::cout << "\n" << "downloading info" << std::endl;

	Download getInfo;
	getInfo.setInputPath(itemName + "/info.dat");
	getInfo.setOutputDir(installDir);
	getInfo.setOutputFile("info.dat");
	getInfo.download();
	getInfo.save();
}

void Item::downloadFiles()
{
	std::cout << "\n" << "downloading files" << std::endl;

	Download getInfo;
	getInfo.setInputPath(itemName + "/release.zip");
	getInfo.setOutputDir(installDir);
	getInfo.setOutputFile("release.zip");
	getInfo.download();
	getInfo.save();

	downloaded = true;
}

void Item::deleteIcon()
{
	std::cout << "deleting icon... ";

	try
	{
		fs::remove(installDir + "icon.png");
		std::cout << "done" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "\n" << e.what() << std::endl;
	}
}

void Item::deleteInfo()
{
	std::cout << "deleting info... ";

	try
	{
		fs::remove(installDir + "info.dat");
		std::cout << "done" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "\n" << e.what() << std::endl;
	}
}
