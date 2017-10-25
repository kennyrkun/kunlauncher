#include "LauncherUpdater.hpp"

#include "Globals.hpp"
#include "Download.hpp"

#include <iostream>
#include <fstream>
#include <experimental\filesystem>

namespace fs = std::experimental::filesystem;

std::string LauncherUpdater::getRemoteVersion()
{
	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	remoteVersion = getHoHouse.fileBuffer;
	return remoteVersion;
}

std::string LauncherUpdater::getLocalVersion()
{
	localVersion = GBL::VERSION;
	return localVersion;
}

int LauncherUpdater::checkForUpdates()
{
	std::cout << "r" << getRemoteVersion() << " : " << "l" << getLocalVersion() << std::endl;

	if (getRemoteVersion() != getLocalVersion())
	{
		std::cout << "launcher is out of date" << std::endl;
		//		return true;
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
	getNewWhorehouse.setInputPath("latest.noexe");
	getNewWhorehouse.setOutputDir(".\\");
	getNewWhorehouse.setOutputFilename("latest_kunlauncher.exe");
	getNewWhorehouse.download();
	getNewWhorehouse.save();

	return getNewWhorehouse.htmlReturnCode;
}

int LauncherUpdater::replaceOldExecutable()
{
	try
	{
		fs::rename("kunlauncher.exe", "kunlaucher.exe.old");
		fs::rename("latest_kunlauncher.exe", "kunlauncher.exe");

		return Status::Success;
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		abort();

		// TODO: handle this better;
		return Status::FailGeneric;
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

			return Status::FailGeneric;
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
		return Status::NoUpdateFile;
	}

	return Status::Success;
}
