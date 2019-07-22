#include "LauncherUpdater.hpp"

#include "Download.hpp"
#include "SettingsParser.hpp"

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

LauncherUpdater::LauncherUpdater()
{
	getRemoteVersion();
}

GBL::LauncherVersion LauncherUpdater::getRemoteVersion()
{
	Download getVersion;
	getVersion.setInput(GBL::WEB::LATEST::DIR + "version.info");
	getVersion.download();

	SettingsParser parseVersion;
	parseVersion.loadFromFile(GBL::DIR::cache + "/version/latest/version.info");

	GBL::LauncherVersion version;

	parseVersion.get("major", version.major);
	parseVersion.get("minor", version.minor);
	parseVersion.get("patch", version.patch);

	parseVersion.get("required", requiredUpdate);

	return remoteVersion = version;
}

int LauncherUpdater::checkForUpdates()
{
	std::cout << "[UPD] " <<
		"r" << remoteVersion.major << "." << remoteVersion.minor << "." << remoteVersion.patch <<
		" : " << 
		"l" << localVersion.major << "." << localVersion.minor << "." << localVersion.patch << std::endl;

	if (requiredUpdate)
		return Status::RequiredUpdate | Status::UpdateAvailable;

	if (remoteVersion > localVersion)
		return Status::UpdateAvailable;

	return Status::NoUpdateAvailable;
}

int LauncherUpdater::downloadUpdate()
{
	Download getNewWhorehouse;
	getNewWhorehouse.setInput(GBL::WEB::LATEST::EXECUTABLE);
	getNewWhorehouse.setOutputDir(".//");
	getNewWhorehouse.setOutputFilename("latest_kunlauncher.exe");

	int status = getNewWhorehouse.download();

	if (status == Download::Status::Success)
	{
		getNewWhorehouse.save();
		return Status::Success;
	}
	else
		return Status::DownloadFailed | Status::Failure;
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
