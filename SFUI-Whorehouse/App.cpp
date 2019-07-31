#include "App.hpp"

#include "Download.hpp"
#include "Globals.hpp"
#include "MessageBox.hpp"
#include "SettingsParser.hpp"

#include <SFML/Network.hpp>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

/*
App::App(int appid, float xSize, float ySize, float xPos, float yPos)
{
	sf::Clock itemCreateTimer;

	std::cout << "creating new card for \"" + std::to_string(appid) + "\"" << std::endl;

	info.appid = appid;
	itemInstallDir = GBL::DIR::apps + std::to_string(info.appid) + "/"; // .//bin//apps//itemName//

	if (!fs::exists(itemInstallDir))
		std::cerr << "app directory does not already exist, creating..." << std::endl;

	if (!fs::exists(itemInstallDir + "icon.png"))
		std::cerr << "icon was not found" << std::endl;

	if (!fs::exists(itemInstallDir + "info.dat"))
		std::cerr << "info was not found" << std::endl;

	parseInfo(itemInstallDir);

	if (!fs::exists(itemInstallDir + "release.zip"))
		std::cout << "release was not found, not installed" << std::endl;

	cardShape.setSize(sf::Vector2f(xSize, 75));
	cardShape.setOrigin(sf::Vector2f(cardShape.getLocalBounds().width / 2, cardShape.getLocalBounds().height / 2));
	cardShape.setPosition(sf::Vector2f(xPos, yPos)); // probably not the best
	cardShape.setFillColor(GBL::theme.palatte.APP_CARD);

	std::cout << "card is ready (took " << itemCreateTimer.getElapsedTime().asSeconds() << " seconds)" << std::endl;
}
*/

bool App::checkForUpdate(sf::Ftp& ftp)
{
	std::cout << "checking for updates" << std::endl;

	sf::Ftp::Response response = ftp.sendCommand("SIZE", GBL::WEB::APPS + std::to_string(info.appid) + "/release.zip");
	if (response.isOk())
	{
		size_t remoteFileSize = std::stoi(response.getMessage());
		size_t fileSize = fs::file_size(GBL::DIR::apps + std::to_string(info.appid) + "/release.zip");

		if (fileSize != remoteFileSize)
		{
			std::cout << "update available" << std::endl;
			return true;
		}
	}
	else
	{
		std::cout << "failed to get release filesize" << std::endl;
		std::cerr << response.getMessage() << std::endl;
	}

	return false;
}

void App::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(cardShape, states);
}

// private

std::ifstream& App::GotoLine(std::ifstream& file, unsigned int line)
{
	file.seekg(std::ios::beg); // go to top

	for (size_t i = 0; i < line - 1; ++i) // go to line
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return file; // return line
}

void App::parseInfo(std::string dir) // a lot easier than I thought it would be.
{
	std::cout << "parsing info for " << dir << std::endl;

	if (fs::exists(dir + "info.dat"))
	{
		SettingsParser itemInfo;
		if (itemInfo.loadFromFile(dir + "info.dat"))
		{
			itemInfo.get("appid", info.appid);
			itemInfo.get("name", info.name);
			itemInfo.get("description", info.description);
			itemInfo.get("version", info.version);
			itemInfo.get("github", info.github);
			itemInfo.get("author", info.author);
		}
	}
	else
	{
		std::cerr << "info file does not exist" << std::endl;
	}
}
