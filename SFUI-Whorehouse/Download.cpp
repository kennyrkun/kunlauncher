#include "Download.hpp"
#include "Globals.hpp"

#include <SFML\Network.hpp>
#include <experimental\filesystem>
#include <iostream>
#include <fstream>
#include <cmath>

namespace fs = std::experimental::filesystem;

Download2::Download2(bool silent_)
{
	silent = silent_;

	if (!silent)
		std::cout << "downloader created" << std::endl;
}

Download2::Download2()
{
	if (!silent)
		std::cout << "downloader created" << std::endl;
}

Download2::~Download2()
{
	if (!silent)
		std::cout << "downloader killed" << std::endl;
}

void Download2::setOutputDir(std::string dir)
{
	saveDir = dir;

	if (!silent)
		std::cout << "output set to: " << saveDir << std::endl;

	if (!fs::exists(saveDir))
		createDirectory(saveDir);
}

std::string Download2::getOutputDir()
{
	return saveDir;
}

void Download2::setOutputFilename(std::string file)
{
	saveFile = file;

	if (!silent)
		std::cout << "output file set to: " << saveFile << std::endl;
}

std::string Download2::getOutputFilename()
{
	return saveFile;
}

void Download2::setInput(std::string in)
{
	std::cout << "raw input path: " << in << std::endl;

	input = in;

	std::string temppath = input;

	for (size_t i = 0; i < temppath.length(); i++)
		if (temppath[i] == '\\')
			temppath[i] = '/'; // change all the slashes to forwards so that FTP accepts them

	std::cout << "steralised input path: " << temppath << std::endl;

	std::string filename = temppath;
	std::string::size_type pos = filename.find_last_of("/\\");
	if (pos != std::string::npos)
		filename = filename.substr(pos + 1);
	remoteFilename = filename;

	remoteDirectory = temppath.erase(temppath.length() - remoteFilename.length(), temppath.back());

	std::cout << "remoteDirectory: " << remoteDirectory << std::endl;
	std::cout << "remoteFilename : " << remoteFilename << std::endl;
}

std::string Download2::getInput()
{
	return input;
}

void Download2::setInputDirectory(std::string directory)
{
	remoteDirectory = directory;
}

std::string Download2::getInputDirectory()
{
	return remoteDirectory;
}

void Download2::setInputFilename(std::string filename)
{
	remoteFilename = filename;
}

std::string Download2::getInputFilename()
{
	return remoteFilename;
}

// TODO: make this function useable, by adding some sort of decryption thingy
uintmax_t Download2::getFileSize()
{
	sf::Ftp ftp;

	// Connect to the server
	sf::Ftp::Response response = ftp.connect("ftp://ftp.myserver.com");
	if (response.isOk())
		if (!silent)
			std::cout << "Connected" << std::endl;

	// Log in
	response = ftp.login("laurent", "dF6Zm89D");
	if (response.isOk())
		if (!silent)
			std::cout << "Logged in" << std::endl;

	response = ftp.sendCommand("SIZE", input);
	if (response.isOk())
		if (!silent)
			std::cout << "File size: " << response.getMessage() << std::endl;

	// Disconnect from the server (optional)
	ftp.disconnect();

	if (response.isOk())
		return std::stoi(response.getMessage());
	else
		return -1;
}

int Download2::download()
{
	std::cout << "saveDir : " << saveDir << std::endl;
	std::cout << "saveFile: " << saveFile << std::endl;
	std::cout << "remoteDirectory: " << remoteDirectory << std::endl;
	std::cout << "remoteFilename : " << remoteFilename << std::endl;

	sf::Ftp ftp;

	sf::Ftp::Response response = ftp.connect("files.000webhost.com");
	if (response.isOk())
		std::cout << "connected to ftp" << std::endl;

	response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
	if (response.isOk())
		std::cout << "Logged in" << std::endl;

	response = ftp.changeDirectory("public_html");
	if (!response.isOk())
	{
		std::cout << "failed to set ftp directory" << std::endl;

		return -1;
	}

	if (fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory + remoteFilename))
	{
		std::cout << "file already exists in cache, removing." << std::endl;

		try
		{
			fs::remove(".\\" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory + remoteFilename);
		}
		catch (const std::exception& e)
		{
			std::cout << "failed to remove already existing file, aborting download:" << std::endl;
			std::cout << e.what() << std::endl;
			return -1;
		}
	}

	std::cout << "dl: " << remoteDirectory + remoteFilename << std::endl;

	if (!fs::exists(".\\" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory))
		createDirectory(".\\" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory);

	response = ftp.download(remoteDirectory + remoteFilename, ".\\" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory);
	if (response.isOk())
	{
		std::cout << response.getStatus() << ": downloaded the thing to the place" << std::endl;

		if (saveFile.empty())
			saveFile = remoteFilename;

		std::cout << "reading into filebuffer" << std::endl;

		std::ifstream fileContent(".\\bin\\cache\\" + remoteDirectory + remoteFilename, std::ios::binary);
		if (fileContent)
		{
			fileBuffer = (std::string((std::istreambuf_iterator<char>(fileContent)), std::istreambuf_iterator<char>()));

			std::cout << "downloaded file, wrote to buffer: " << getAppropriateFileSize(fileBuffer.size(), 3) << std::endl;
			fileSize = fileBuffer.size();
		}
		else
		{
			std::cout << "failed to open saved file" << std::endl;
		}
	}
	else
	{
		std::cout << response.getMessage() << ": something went wrong! (" << response.getStatus() << ")" << std::endl;
	}

	// Disconnect from the server (optional)
	ftp.disconnect();

	htmlReturnCode = sf::Http::Response::Status::Ok;
	return htmlReturnCode;
}

void Download2::save()
{
	try
	{
		if (fs::exists(saveDir + saveFile))
		{
			std::cout << "output file already exists, overwriting" << std::endl;
			fs::remove(saveDir + saveFile);
		}

		fs::copy_file(".\\" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory + remoteFilename, saveDir + saveFile);
		std::cout << "saved file" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << "failed to save file:" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

std::string Download2::getAppropriateFileSize(const long long int bytes, const int decimals)
{
	int bytesPerUnit = 1024;
	std::string sizes[] = { "Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "REALLY FUCKIN BIG" };
	int i = std::floor(std::log(bytes) / std::log(bytesPerUnit));

	std::string number = std::to_string(bytes / std::pow(bytesPerUnit, i));
	number.erase(number.find('.') + decimals + 1, number.length());
	return number + " " + sizes[i];
}

void Download2::clearCache()
{
	try
	{
		fs::remove(".\\" + GBL::DIR::BASE + GBL::DIR::CACHE);
		std::cout << "download cache cleared" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "failed to clear download cache:" << std::endl;
		std::cout << e.what() << std::endl;
	}
}

// private:

void Download2::createDirectory(std::string dir) // recursively
{
	std::cout << "creating " << dir << std::endl;

	std::vector<fs::path> subdirectories;
//	dir.erase(0, 2); // .\\

	for (auto& part : fs::path(dir))
	{
		subdirectories.push_back(part);
	}

	fs::path last = subdirectories.front(); // first path
	for (size_t i = 0; i < subdirectories.size() - 1; i++)
	{
		fs::create_directory(last);
		last = last.append(subdirectories[i + 1]); // add the next path to this path
	}
}
