#include "LauncherUpdater.hpp"

#include "Download.hpp"
#include "SettingsParser.hpp"

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

GBL::Version LauncherUpdater::getRemoteVersion()
{
	Download getHoHouse;
	getHoHouse.setInput(GBL::WEB::LATEST::DIR + "version.info");
	getHoHouse.download();

	SettingsParser getVersion;
	getVersion.loadFromFile(GBL::DIR::cache + "/version/latest/version.info");

	GBL::Version version;

	getVersion.get("major", version.major);
	getVersion.get("minor", version.minor);
	getVersion.get("patch", version.patch);

	getVersion.get("required", requiredUpdate);

	return remoteVersion = version;
}

GBL::Version& LauncherUpdater::getLocalVersion()
{
	return localVersion = GBL::version;
}

int LauncherUpdater::checkForUpdates()
{
	getRemoteVersion();
	getLocalVersion();

	std::cout << "[UPD] " <<
		"r" << remoteVersion.major << "." << remoteVersion.minor << "." << remoteVersion.patch << 
		" : " << 
		"l" << localVersion.major << "." << localVersion.minor << "." << localVersion.patch << std::endl;

	if (requiredUpdate)
		return Status::RequiredUpdate;

	if (remoteVersion.major > localVersion.major)
		return Status::UpdateAvailable;

	if (remoteVersion.minor > localVersion.minor)
		return Status::UpdateAvailable;

	if (remoteVersion.patch > localVersion.patch)
		return Status::UpdateAvailable;

	return Status::NoUpdateAvailable;
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
		std::cerr << "[UPD] failed to replace old executable" << std::endl;

		return Status::Failure;
	}
}

int LauncherUpdater::removeOldExecutable()
{
	std::cout << "[UPD] removing old executable" << std::endl;

	if (fs::exists("kunlauncher.exe.old"))
	{
		try
		{
			fs::remove("kunlauncher.exe.old");
			std::cout << "[UPD] removed old executable" << std::endl;
			return Status::Success;
		}
		catch (const std::exception& e)
		{
			std::cout << "[UPD] could not remvoe old executable:" << std::endl;
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
