#include "LauncherUpdater.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "SettingsParser.hpp"

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

int LauncherUpdater::getRemoteVersion()
{
	Download getHoHouse;
	getHoHouse.setInput(GBL::WEB::LATEST::DIR + "version.info");
	getHoHouse.download();

	SettingsParser getVersion;
	getVersion.loadFromFile(GBL::DIR::cache + "/launcher/version/latest/version.info");

	getVersion.get("version", remoteVersion);
	getVersion.get("required", requiredUpdate);

	return remoteVersion;
}

int LauncherUpdater::getLocalVersion()
{
	return localVersion = GBL::VERSION;
}

int LauncherUpdater::checkForUpdates()
{
	getRemoteVersion();
	getLocalVersion();

	std::cout << "r" << remoteVersion << " : " << "l" << localVersion << std::endl;

	if (remoteVersion > localVersion)
	{
		if (requiredUpdate)
			return Status::RequiredUpdate;

		std::cout << "launcher is out of date" << std::endl;
		return Status::UpdateAvailable;
	}
	else
	{
		std::cout << "launcher is up to date or broken" << std::endl;
		return Status::NoUpdateAvailable;
	}
}

int LauncherUpdater::downloadUpdate()
{
	Download getNewWhorehouse;
	getNewWhorehouse.setInput(GBL::WEB::LATEST::EXECUTABLE);
	getNewWhorehouse.setOutputDir(".//");
	getNewWhorehouse.setOutputFilename("latest_kunlauncher.exe");
	getNewWhorehouse.download();
	getNewWhorehouse.save();

	return getNewWhorehouse.htmlReturnCode;
}

int LauncherUpdater::replaceOldExecutable()
{
	try
	{
		if (fs::exists("kunlauncher.exe"))
			fs::rename("kunlauncher.exe", "kunlauncher.exe.old");

		if (fs::exists("latest_kunlauncher.exe"))
			fs::rename("latest_kunlauncher.exe", "kunlauncher.exe");

		return Status::Success;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		std::cerr << "failed to replace old executable" << std::endl;

		return Status::Failure;
	}
}

int LauncherUpdater::removeOldExecutable()
{
	std::cout << "removing old executable" << std::endl;

	if (fs::exists("kunlauncher.exe.old"))
	{
		try
		{
			fs::remove("kunlauncher.exe.old");
			std::cout << "removed old executable" << std::endl;
			return Status::Success;
		}
		catch (const std::exception& e)
		{
			std::cout << "could not remvoe old executable:" << std::endl;
			std::cout << e.what() << std::endl;

			return Status::Failure;
		}
	}

	return 0;
}

/* method to seperate version numbers

std::string newpatchs, newminors, newmajors;
int newpatch, newminor, newmajor;

const std::string old = remoteVersion;

newmajors = remoteVersion.erase(remoteVersion.find_first_of('.'), remoteVersion.back());
newmajor = std::stoi(newmajors);

remoteVersion = old;

remoteVersion.erase(0, remoteVersion.find_first_of('.') + 1);
remoteVersion.erase(remoteVersion.find_first_of('.'), remoteVersion.back());
newminors = remoteVersion;
newminor= std::stoi(newminors);

remoteVersion = old;

remoteVersion.erase(0, remoteVersion.find_last_of('.') + 1);
newpatchs = remoteVersion;
newpatch = std::stoi(newpatchs);
*/
