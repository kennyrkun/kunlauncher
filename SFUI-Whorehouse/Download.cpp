#include "Download.hpp"
#include "Globals.hpp"

#include <SFML/Network.hpp>
#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <cmath>

namespace fs = std::experimental::filesystem;

Download::Download()
{
	std::cout << "downloader created" << std::endl;
}

Download::~Download()
{
	std::cout << "downloader killed" << std::endl;
}

void Download::setOutputDir(std::string dir)
{
	saveDir = dir;

	std::cout << "output set to: " << saveDir << std::endl;

	if (!fs::exists(saveDir))
		createDirectory(saveDir);
}

std::string Download::getOutputDir()
{
	return saveDir;
}

void Download::setOutputFilename(std::string file)
{
	saveFile = file;

	std::cout << "output file set to: " << saveFile << std::endl;
}

std::string Download::getOutputFilename()
{
	return saveFile;
}

void Download::setInput(std::string in)
{
//	std::cout << "raw input path: " << in << std::endl;

	input = in;

	std::string temppath = input;

	for (size_t i = 0; i < temppath.length(); i++)
		if (temppath[i] == '//')
			temppath[i] = '/'; // change all the slashes to forwards so that FTP accepts them

	std::cout << "steralised input path: " << temppath << std::endl;

	std::string filename = temppath;
	std::string::size_type pos = filename.find_last_of("/\\");
	if (pos != std::string::npos)
		filename = filename.substr(pos + 1);
	remoteFilename = filename;

	remoteDirectory = temppath.erase(temppath.length() - remoteFilename.length(), temppath.back());

//	std::cout << "remoteDirectory: " << remoteDirectory << std::endl;
//	std::cout << "remoteFilename : " << remoteFilename << std::endl;
}

std::string Download::getInput()
{
	return input;
}

void Download::setInputDirectory(std::string directory)
{
	remoteDirectory = directory;
}

std::string Download::getInputDirectory()
{
	return remoteDirectory;
}

void Download::setInputFilename(std::string filename)
{
	remoteFilename = filename;
}

std::string Download::getInputFilename()
{
	return remoteFilename;
}

// TODO: make this function useable, by adding some sort of decryption thingy
uintmax_t Download::getFileSize()
{
	sf::Ftp ftp;

	// Connect to the server
	sf::Ftp::Response response = ftp.connect("ftp://ftp.myserver.com");
	if (response.isOk())
		std::cout << "Connected" << std::endl;

	// Log in
	response = ftp.login("laurent", "dF6Zm89D");
	if (response.isOk())
		std::cout << "Logged in" << std::endl;

	response = ftp.sendCommand("SIZE", input);
	if (response.isOk())
		std::cout << "File size: " << response.getMessage() << std::endl;

	// Disconnect from the server (optional)
	ftp.disconnect();

	if (response.isOk())
		return std::stoi(response.getMessage());
	else
		return 0;
}

int Download::download()
{
//	std::cout << "saveDir : " << saveDir << std::endl;
//	std::cout << "saveFile: " << saveFile << std::endl;
//	std::cout << "remoteDirectory: " << remoteDirectory << std::endl;
//	std::cout << "remoteFilename : " << remoteFilename << std::endl;

	sf::Ftp ftp;

	sf::Ftp::Response response = ftp.connect("files.000webhost.com", 21, sf::milliseconds(10000));
	if (!response.isOk())
	{
		std::cerr << "failed to connect to ftp (" << response.getMessage() << " (" << response.getStatus() << "))" << std::endl;
		return Status::ConnectionFailed;
	}

	response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
	if (!response.isOk())
	{
		std::cerr << "failed to login to default ftp (" << response.getMessage() << " (" << response.getStatus() << "))" << std::endl;

		response = ftp.login("kunlauncher2", "9fH^!U2=Ys=+XJYq");
		if (!response.isOk())
		{
			std::cerr << "failed to login to auxilary ftp (" << response.getMessage() << " (" << response.getStatus() << "))" << std::endl;
			return Status::ConnectionRejected;
		}
		else
		{
			std::cout << "connected to aux ftp" << std::endl;
		}
	}

	response = ftp.changeDirectory("public_html");
	if (!response.isOk())
	{
		std::cerr << "failed to set ftp directory" << std::endl;
		return Status::Failure;
	}

	if (fs::exists(".//" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory + remoteFilename))
	{
		std::cout << "file already exists in cache, removing." << std::endl;

		try
		{
			fs::remove(".//" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory + remoteFilename);
		}
		catch (const std::exception& e)
		{
			std::cerr << "failed to remove already existing file" << std::endl;
			std::cerr << e.what() << std::endl;
		}
	}

	if (!fs::exists(".//" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory))
		createDirectory(".//" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory);

	response = ftp.download(remoteDirectory + remoteFilename, ".//" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory);
	if (response.isOk())
	{
		std::cout << response.getStatus() << ": downloaded the thing to " << saveDir << std::endl;

		if (saveFile.empty())
			saveFile = remoteFilename;

		std::ifstream fileContent(".//bin//cache//" + remoteDirectory + remoteFilename, std::ios::binary);
		if (fileContent)
		{
			fileBuffer = (std::string((std::istreambuf_iterator<char>(fileContent)), std::istreambuf_iterator<char>()));

			std::cout << "downloaded file, wrote to buffer: " << getAppropriateFileSize(fileBuffer.size(), 3) << std::endl;
			fileSize = fileBuffer.size();
		}
		else
		{
			std::cerr << "failed to open saved file" << std::endl;
		}
		fileContent.close();
	}
	else
	{
		std::cerr << response.getMessage() << ": something went wrong! (" << response.getStatus() << ")" << std::endl;
		return Status::Failure;
	}

	// Disconnect from the server (optional)
	ftp.disconnect();

	return Status::Ok;
}

int Download::save()
{
	try
	{
		if (fs::exists(saveDir + saveFile))
		{
			std::cout << "output file already exists, overwriting" << std::endl;
			fs::remove(saveDir + saveFile);
		}

		fs::copy_file(".//" + GBL::DIR::BASE + GBL::DIR::CACHE + remoteDirectory + remoteFilename, saveDir + saveFile);
		std::cout << "saved file" << std::endl;

		return Status::Ok;
	}
	catch (const std::exception &e)
	{
		std::cerr << "failed to save file:" << std::endl;
		std::cerr << e.what() << std::endl;

		return Status::SaveFailed;
	}
}

std::string Download::getAppropriateFileSize(const long long int bytes, const int decimals)
{
	int bytesPerUnit = 1024;
	std::string sizes[] = { "Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "REALLY FUCKIN BIG" };
	int i = std::floor(std::log(bytes) / std::log(bytesPerUnit));

	std::string number = std::to_string(bytes / std::pow(bytesPerUnit, i));
	number.erase(number.find('.') + decimals + 1, number.length());
	return number + " " + sizes[i];
}

void Download::clearCache()
{
	try
	{
		fs::remove(".//" + GBL::DIR::BASE + GBL::DIR::CACHE);
		std::cout << "download cache cleared" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "failed to clear download cache:" << std::endl;
		std::cerr << e.what() << std::endl;
	}
}

// private:

void Download::createDirectory(std::string dir) // recursively
{
	std::cout << "creating " << dir << std::endl;

	std::vector<fs::path> subdirectories;
//	dir.erase(0, 2); // .//

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
