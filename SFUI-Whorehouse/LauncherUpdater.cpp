#include "LauncherUpdater.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "SettingsParser.hpp"

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

std::string LauncherUpdater::getRemoteVersion()
{
	Download getHoHouse;
	getHoHouse.setInput(GBL::WEB::LATEST::DIR + "version.info");
	getHoHouse.download();

	remoteVersion = getHoHouse.fileBuffer;

	SettingsParser getVersion;

	return remoteVersion;
}

std::string LauncherUpdater::getLocalVersion()
{
//	Version localVersion = {GBL::VERSION::majors, GBL::VERSION::minors, GBL::VERSION::patchs, GBL::VERSION::major, GBL::VERSION::minor, GBL::VERSION::patch};

	return GBL::VERSION::string;
}

int LauncherUpdater::checkForUpdates()
{
	getRemoteVersion();
	getLocalVersion();

	std::cout << "r" << remoteVersion << " : " << "l" << localVersion << std::endl;

	if (remoteVersion.empty())
	{
		std::cout << "failed to get version info" << std::endl;

		return Status::Failure;
	}

	if (remoteVersion.find("REQUIRED"))
		requiredUpdate = true;

	//TODO: actual version checking
	// if lpatch < rpatch
	//	if lminor < rminor
	//  else
	//	  if lmajor < rmajor
	//		update = true;
	//	  else
	// else
	// if lminor < rminor
	// else
	//	if lmajor < rmajor
	//	   update = true;
	//	else

	if (remoteVersion != localVersion)
	{
		std::cout << "launcher is out of date" << std::endl;
		return Status::UpdateAvailable;
	}
	else
	{
		std::cout << "launcher is up to date" << std::endl;
		return Status::NoUpdateAvailable;
	}
}

int LauncherUpdater::downloadUpdate()
{
	Download getNewWhorehouse;
	getNewWhorehouse.setInput("latest.noexe");
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
			fs::rename("kunlauncher.exe", "kunlaucher.exe.old");

		if (fs::exists("latest_kunlauncher.exe"))
			fs::rename("latest_kunlauncher.exe", "kunlauncher.exe");

		return Status::Success;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		abort();

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

int LauncherUpdater::createUpdateFile()
{
	std::cout << "creating update file" << std::endl;

	std::ofstream writeUpdateFile(GBL::DIR::BASE + "up.date", std::ios::binary);

	if (writeUpdateFile.is_open())
	{
		writeUpdateFile << "OldVersion=\"" << localVersion << "\"" << std::endl;
		writeUpdateFile << "NewVersion=\"" << remoteVersion << "\"" << std::endl;

		writeUpdateFile.close();

		return 0;
	}
	else
	{
		return -1;
	}
}

int LauncherUpdater::readUpdateFile()
{
	if (fs::exists(GBL::DIR::BASE + GBL::DIR::CONFIG + "up.date"))
	{
		std::cout << "reading update file" << std::endl;

		std::string fileBuffer, temp, oldVersion, newVersion, updateFinished;

		std::ifstream getUpdateFile(GBL::DIR::BASE + "up.date", std::ios::binary);
		getUpdateFile >> fileBuffer;

		std::cout << "filebuffer--------" << "\n" << fileBuffer << std::endl;

		temp.erase(0, temp.find('"') + 1); // OlderVersion="
		temp.erase(temp.find('"'), temp.back());
		oldVersion = temp;
		std::cout << "oldVersion: " << oldVersion << std::endl;

		fileBuffer.erase(0, temp.find("\n")); // delete first line
		temp = fileBuffer;

		std::cout << "------------------" << "\n" << fileBuffer << std::endl;
	}
	else
	{
//		return Status::NoUpdateFile;
	}

	return Status::Success;
}
