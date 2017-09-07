#include "Download.hpp"
#include "Globals.hpp"

#include <SFML\Network.hpp>
#include <experimental\filesystem>
#include <iostream>
#include <fstream>
#include <cmath>

Download::Download(bool silent_)
{
	if (silent_)
		silent = true;
	else
		silent = false;

	if (!silent)
		std::cout << "downloader created" << std::endl;
}

Download::Download()
{
	if (!silent)
		std::cout << "downloader created" << std::endl;
}

Download::~Download()
{
	if (!silent)
		std::cout << "downloader killed" << std::endl;
}

void Download::setOutputDir(std::string dir)
{
	outdir = dir;

	if (!silent)
		std::cout << "output set to: " << outdir << std::endl;

	if (!std::experimental::filesystem::exists(outdir))
		createDirectory(outdir);
}

std::string Download::getOutputDir()
{
	return outdir;
}

void Download::setOutputFilename(std::string file)
{
	outfile = file;

	if (!silent)
		std::cout << "output file set to: " << outfile << std::endl;
}

std::string Download::getOutputFilename()
{
	return outfile;
}

void Download::setInputPath(std::string path)
{
	inpath = path;
}

std::string Download::getInputPath()
{
	return inpath;
}

int Download::download()
{
	sf::Http http(CONST::DIR::WEB_HOSTNAME);
	sf::Http::Request request("/" + inpath, sf::Http::Request::Get);

	sf::Clock timer;
	sf::Http::Response response = http.sendRequest(request, sf::seconds(10));
	float elapsedTime = timer.getElapsedTime().asSeconds();

	fileSize = response.getBody().size();
	if (!silent)
		std::cout << "downloading remote (" << fileSize << "b (" << getAppropriateFileSize(fileSize, 2) << "))... " << std::endl;
	 
	fileBuffer = response.getBody();
	if (!silent)
		std::cout << "done downloading. (took " << elapsedTime << " seconds.)" << std::endl;

	switch (response.getStatus())
	{
	case sf::Http::Response::Ok:
		if (!silent)
			std::cout << "successfully connected to file server, and got files" << std::endl;

		return sf::Http::Response::Status::Ok;
		break;

	case sf::Http::Response::NotFound:
		if (!silent)
			std::cout << "file does not exist on remote server (404)" << std::endl;

		return sf::Http::Response::Status::NotFound;
		break;

	case sf::Http::Response::InternalServerError:
		if (!silent)
			std::cout << "internal server error was encountered, aborting..." << std::endl;

		return sf::Http::Response::Status::InternalServerError;
		break;

	default:
		break;
	}
}

void Download::save()
{
	std::ofstream downloadFile(outdir + outfile, std::ios::out | std::ios::binary);
	if (!silent)
		std::cout << "saving file to \"" << outdir << outfile << "\"... " << std::endl;

	for (int i = 0; i < fileSize; i++)
		downloadFile << fileBuffer[i];
	downloadFile.close();

	if (downloadFile.fail())
		if (!silent)
			std::cout << "failed" << std::endl;
	else
		if (!silent)
			std::cout << "finished" << std::endl;
}

std::string Download::getAppropriateFileSize(const long long int bytes, const int decimals)
{
	int bytesPerUnit = 1024;
	std::string sizes[] = { "Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "UNDEFINED" };
	int i = std::floor(std::log(bytes) / std::log(bytesPerUnit));

	std::string number = std::to_string(bytes / std::pow(bytesPerUnit, i));
	number.erase(number.find('.') + decimals + 1, number.length());
	return number + " " + sizes[i];
}

// private:

void Download::createDirectory(std::string dir)
{
//	std::cout << "creating " << dir << "... ";

	std::vector<std::experimental::filesystem::path> subdirectories;
	dir.erase(0, 2); // .\\

	for (auto& part : std::experimental::filesystem::path(dir))
	{
		subdirectories.push_back(part);
	}

	std::experimental::filesystem::path last = subdirectories.front(); // first path
	for (size_t i = 0; i < subdirectories.size() - 1; i++)
	{
		std::experimental::filesystem::create_directory(last);
		last = last.append(subdirectories[i + 1]); // add the next path to this path
	}

//	std::cout << "done." << std::endl;
}
