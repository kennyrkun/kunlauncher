#include "Download.hpp"
#include "Globals.hpp"

#include <SFML\Network.hpp>
#include <experimental\filesystem>
#include <iostream>
#include <fstream>
#include <cmath>

Download::Download(bool silent_)
{
	silent = silent_;

	if (!silent)
		std::cout << "downloader created" << "\n";
}

Download::Download()
{
	if (!silent)
		std::cout << "downloader created" << "\n";
}

Download::~Download()
{
	if (!silent)
		std::cout << "downloader killed" << "\n";
}

void Download::setOutputDir(std::string dir)
{
	outdir = dir;

	if (!silent)
		std::cout << "output set to: " << outdir << "\n";

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
		std::cout << "output file set to: " << outfile << "\n";
}

std::string Download::getOutputFilename()
{
	return outfile;
}

void Download::setInputPath(std::string path)
{
	inpath = path;

	if (!silent)
		std::cout << "input uri set to: " << inpath << "\n";
}

std::string Download::getInputPath()
{
	return inpath;
}

int Download::download()
{
	sf::Http http(CONST::DIR::WEB_HOSTNAME);
	sf::Http::Request request(inpath, sf::Http::Request::Get);

	sf::Clock timer;
	sf::Http::Response response = http.sendRequest(request, sf::seconds(10));
	float elapsedTime = timer.getElapsedTime().asSeconds();

	fileBuffer = response.getBody();
	fileSize = response.getBody().size();

	if (!silent)
	{
		std::cout << "downloaded remote (" << fileSize << "b (" << getAppropriateFileSize(fileSize, 2) << "))... " << "\n";
		std::cout << "download took " << elapsedTime << " seconds" << "\n";
	}

	htmlReturnCode = response.getStatus();

	switch (response.getStatus())
	{
	case sf::Http::Response::Ok:
		if (!silent)
			std::cout << "successfully connected to file server, and got files (200)" << "\n";

		return sf::Http::Response::Status::Ok;
		break;

	case sf::Http::Response::NotFound:
		if (!silent)
			std::cout << "file does not exist on remote server (404)" << "\n";

		return sf::Http::Response::Status::NotFound;
		break;

	case sf::Http::Response::InternalServerError:
		if (!silent)
			std::cout << "encountered Internal Server Error (500)" << "\n";

		return sf::Http::Response::Status::InternalServerError;
		break;

	default:
		return sf::Http::Response::Status::ResetContent;
		break;
	}
}

void Download::save()
{
	std::ofstream downloadFile(outdir + outfile, std::ios::out | std::ios::binary);
	
	if (downloadFile.is_open())
	{
		if (!silent)
			std::cout << "saving file to \"" << outdir << outfile << "\"... " << "\n";

		for (int i = 0; i < fileSize; i++)
			downloadFile << fileBuffer[i];
		downloadFile.close();

		if (downloadFile.fail())
			if (!silent)
				std::cout << "failed" << "\n";
		else
			if (!silent)
				std::cout << "finished" << "\n";
	}
	else
	{
		std::cout << "failed to open file for saving" << std::endl;

		return;
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

// private:

void Download::createDirectory(std::string dir) // recursively
{
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
}
