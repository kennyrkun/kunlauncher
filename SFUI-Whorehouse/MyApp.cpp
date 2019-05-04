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
	name.setFillColor(GBL::theme.palatte.TEXT_DARK);

	description.setFont(font);
	description.setCharacterSize(16);
	description.setFillColor(GBL::theme.palatte.TEXT_DARK);

	version.setFont(font);
	version.setCharacterSize(18);
	version.setFillColor(GBL::theme.palatte.TEXT_DARK);

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

int MyApp::onClick(sf::Event &e, sf::Vector2f clickPos)
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

bool MyApp::checkForUpdate()
{
	std::cout << "checking for updates" << std::endl;

	Download getRemoteVersion;
	getRemoteVersion.setInput("./" + GBL::WEB::APPS + std::to_string(info.appid) + "/info.dat");
	getRemoteVersion.download();

	int rRelease = 0, lRelease = info.release;

	SettingsParser getVersion;
	if (getVersion.loadFromFile("./" + GBL::DIR::cache + "apps/" + std::to_string(info.appid) + "/info.dat"))
		getVersion.get("release", rRelease);
	else
	{
		std::cerr << "failed to get remote app version" << std::endl;
		return false;
	}

	if (lRelease < rRelease)
	{
		std::cout << "item is out of date! (local: " << lRelease << " : remote: " << rRelease << ")" << std::endl;

		updateIsAvailable = true;
		redownloadButton.setFillColor(sf::Color::Yellow);

		return true;
	}
	else
	{
		std::cout << "item is up to date or newer! :D (local: " << lRelease << " : remote: " << rRelease << ")" << std::endl;

		return false;
	}
}

void MyApp::redownload()
{
	std::cout << "updating item" << std::endl;

	download();
}

void MyApp::download()
{
	info.downloading = true;

	float fuckedUpXPosition = (cardShape.getPosition().x + cardShape.getLocalBounds().width) - 30;
	redownloadButton.setOrigin(sf::Vector2f(redownloadButton.getLocalBounds().width / 2, redownloadButton.getLocalBounds().height / 2));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + 37.f));
	info.missingInfo = true;

	if (fs::exists(itemInstallDir + "/release.zip"))
	{
		std::cout << "downloading " << info.name << std::endl;

		downloadIcon();
		downloadInfo();
		downloadFiles();

		std::cout << "\n" << "finished downloading " << info.name << std::endl;
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

	info.downloading = false;

	redownloadButton.setRotation(30);
	redownloadButton.setOrigin(sf::Vector2f(0, 0));
	redownloadButton.setPosition(sf::Vector2f(fuckedUpXPosition + 7, cardShape.getPosition().y + 10));
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
	target.draw(cardShape);

//	target.draw(controlBar);
	target.draw(icon);

	target.draw(name);
	target.draw(description);
	target.draw(version);

	if (info.downloading)
		target.draw(redownloadButton);
	else // not downloading
		if (info.downloaded)
		{
			target.draw(redownloadButton);
			target.draw(removeButton);
			target.draw(launchButton);
		}
		else
			if (!info.missingInfo)
				target.draw(downloadButton);
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

		iconTexture.loadFromFile(GBL::DIR::textures + "error_2x.png");
		updateIsAvailable = true; // because there's no way to represent an error yet, we just mark it for needing to be redownloaded

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

	Download getFiles;
	getFiles.setInput(GBL::WEB::APPS + std::to_string(info.appid) + "/release.zip");
	getFiles.setOutputDir(GBL::DIR::apps + std::to_string(info.appid) + "//");
	getFiles.setOutputFilename("release.zip");
	getFiles.download();
	getFiles.save();

	info.downloaded = true;

	return 1;
}
