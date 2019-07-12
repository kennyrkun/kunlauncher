#include "Download.hpp"
#include "Globals.hpp"

#include <experimental/filesystem>
#include <iostream>
#include <fstream>
#include <cmath>

namespace fs = std::experimental::filesystem;

Download::Download()
{
	std::cout << "[DL] downloader created" << std::endl;
}

Download::~Download()
{
	std::cout << "[DL] downloader killed" << std::endl;
}

void Download::setOutputDir(std::string dir)
{
	saveDir = dir;

	std::cout << "[DL] output set to: " << saveDir << std::endl;

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

	std::cout << "[DL] output file set to: " << saveFile << std::endl;
}

std::string Download::getOutputFilename()
{
	return saveFile;
}

void Download::setInput(std::string in)
{
	std::string temppath = in;

	for (size_t i = 0; i < temppath.length(); i++)
		if (temppath[i] == '//')
			temppath[i] = '/'; // change all the slashes to forwards so that FTP accepts them

	std::cout << "[DL] sterilized input path: " << temppath << std::endl;

	std::string filename = temppath;
	std::string::size_type pos = filename.find_last_of("/\\");
	if (pos != std::string::npos)
		filename = filename.substr(pos + 1);
	remoteFilename = filename;

	remoteDirectory = temppath.erase(temppath.length() - remoteFilename.length(), temppath.back());
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

uintmax_t Download::getFileSize()
{
// TODO: make this function useable, by adding some sort of decryption thingy
	sf::Ftp ftp;
	
	if (!downloaded && !remoteFilename.empty())
	{
		std::cout << "[DL] retrieving file size" << std::endl;

		// Connect to the server
		sf::Ftp::Response response = ftp.connect("files.000webhost.com");
		if (response.isOk())
			std::cout << "[DL] Connected" << std::endl;

		// Log in
		response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
		if (response.isOk())
			std::cout << "[DL] Logged in" << std::endl;

		std::cout << "[DL] remote directory + remote filename: " << remoteDirectory + remoteFilename << std::endl;

		response = ftp.sendCommand("SIZE", remoteDirectory + remoteFilename);
		if (response.isOk())
			std::cout << "[DL] File size: " << response.getMessage() << std::endl;
		else
			std::cerr << response.getMessage() << std::endl;

		ftp.disconnect();

		if (response.isOk())
			return std::stoi(response.getMessage());
		else
			return 0;
	}
	else
	{
		return fileSize;
	}
}

int Download::download()
{
//	std::cout << "saveDir : " << saveDir << std::endl;
//	std::cout << "saveFile: " << saveFile << std::endl;
//	std::cout << "remoteDirectory: " << remoteDirectory << std::endl;
//	std::cout << "remoteFilename : " << remoteFilename << std::endl;

	sf::Ftp ftp;

	downloaded = false;
	sizeVerified = false;

	sf::Ftp::Response response = ftp.connect("files.000webhost.com", 21, sf::milliseconds(10000));
	if (!response.isOk())
	{
		std::cerr << "[DL] failed to connect to ftp (" << response.getMessage() << " (" << response.getStatus() << "))" << std::endl;
		return Status::Fail | Status::ConnectionFailed;
	}

	response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
	if (!response.isOk())
	{
		std::cout << "[DL] failed to login to ftp" << std::endl;
		return Status::Fail | Status::LoginFailed;
	}

	response = ftp.changeDirectory("public_html");
	if (!response.isOk())
	{
		std::cerr << "[DL] failed to set ftp directory" << std::endl;
		return Status::Fail;
	}

	// TODO: sometimes this, we fail to download and get 500 RETR.
	// since the download happens after this, there will be no resource
	// manifest until a new one is downloaded (usually after a restart)
	// SOLUTION: instead, we should rename the file and then delete it later
	if (fs::exists(GBL::DIR::cache + remoteDirectory + remoteFilename))
	{
		std::cout << "[DL] file already exists in cache, removing." << std::endl;

		try
		{
			fs::remove(GBL::DIR::cache + remoteDirectory + remoteFilename);
		}
		catch (const std::exception& e)
		{
			std::cerr << "[DL] failed to remove already existing file" << std::endl;
			std::cerr << e.what() << std::endl;
		}
	}

	if (!fs::exists(GBL::DIR::cache + remoteDirectory))
		createDirectory(GBL::DIR::cache + remoteDirectory);

	// we use this to verify that the download was at least full
	response = ftp.sendCommand("SIZE", remoteDirectory + remoteFilename);
	if (response.isOk())
	{
		std::cout << "[DL] remote file size: " << response.getMessage() << std::endl;
		fileSize = std::stoi(response.getMessage());
	}
	else
	{
		std::cerr << "[DL] (" << response.getStatus() << ") " << response.getMessage() << std::endl;
	}

	std::cout << "[DL] BEGIN DOWNLOAD" << std::endl;

	response = ftp.download(remoteDirectory + remoteFilename, GBL::DIR::cache + remoteDirectory, sf::Ftp::TransferMode::Binary);
	if (response.isOk())
	{
		std::cout << "[DL] " << response.getStatus() << ": download success, saved to " << saveDir << std::endl;

		if (saveFile.empty())
			saveFile = remoteFilename;

		// TODO: possibly don't overwrite file
		std::ifstream fileContent(GBL::DIR::cache + remoteDirectory + remoteFilename, std::ios::binary, std::ios::trunc);
		if (fileContent.is_open())
		{
			fileBuffer = (std::string((std::istreambuf_iterator<char>(fileContent)), std::istreambuf_iterator<char>()));

			std::cout << "[DL] wrote file to to buffer: " << getAppropriateFileSize(fileBuffer.size(), 3) << std::endl;
			fileSize = fileBuffer.size();

			fileContent.close();

			// verify file size
			if (fileSize != fs::file_size(GBL::DIR::cache + remoteDirectory + remoteFilename))
			{
				std::cerr << "[DL] downloaded file size differs from file size reported by server!" << std::endl;
				sizeVerified = true;
			}
		}
		else
		{
			std::cerr << "[DL] failed to open file for reading into buffer" << std::endl;
			return Status::Fail;
		}
	}
	else
	{
		std::cerr << "[DL] " << "(" << response.getStatus() << ")" << response.getMessage() << std::endl;
		return Status::Fail;
	}

	ftp.disconnect();
	downloaded = true;

	return Status::Success;
}

int Download::save()
{
	try
	{
		if (!fs::exists(GBL::DIR::cache + remoteDirectory + remoteFilename))
		{
			std::cerr << "[DL] downloaded file does not exist in cache" << std::endl;
			return Status::Fail;
		}

		if (!downloaded)
		{
			std::cerr << "[DL] download status was not set as true!" << std::endl;
			return Status::Fail;
		}

		if (fs::exists(saveDir + saveFile))
		{
			std::cout << "[DL] output file already exists, overwriting" << std::endl;
			fs::remove(saveDir + saveFile);
		}

		fs::copy_file(GBL::DIR::cache + remoteDirectory + remoteFilename, saveDir + saveFile);
		std::cout << "[DL] saved file" << std::endl;

		return Status::Success;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[DL] failed to save file:" << std::endl;
		std::cerr << e.what() << std::endl;

		return Status::Fail;
	}
}

std::string Download::getAppropriateFileSize(const long long int bytes, const int decimals)
{
	int bytesPerUnit = 1024;
	std::string sizes[] = { "Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "REALLY FUCKIN BIG" };
	int i = std::floor(std::log(bytes) / std::log(bytesPerUnit));
	// TODO: look into changing int to double

	std::string number = std::to_string(bytes / std::pow(bytesPerUnit, i));
	number.erase(number.find('.') + decimals + 1, number.length());
	return number + " " + sizes[i];
}

void Download::clearCache()
{
	try
	{
		fs::remove_all(GBL::DIR::cache);
		std::cout << "[DL] upload cache cleared" << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[DL] failed to clear upload cache:" << std::endl;
		std::cerr << e.what() << std::endl;
	}
}

// private:

void Download::createDirectory(std::string dir) // recursively
{
	std::cout << "[DL] creating " << dir << std::endl;

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
