#include "Download.hpp"

#include <SFML\Network.hpp>
#include <experimental\filesystem>
#include <iostream>
#include <fstream>
#include <cmath>

Download::Download(bool)
{
	silent = true;

	// nothing
}

Download::Download()
{
	silent = false;

	// nothing
}

Download::~Download()
{
	// nothing
}

void Download::setOutputDir(std::string dir)
{
	outdir = dir;

	if (!silent)
		std::cout << "output set to: " << outdir << std::endl;

	if (!exists(outdir))
		createDirectory(outdir);
}

std::string Download::getOutputDir()
{
	return outdir;
}

void Download::setOutputFile(std::string file)
{
	outfile = file;
//	std::cout << "output file set to: " << outfile << std::endl;
}

std::string Download::getOutputFile()
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
	sf::Clock timer;

	sf::Http http("kunlauncher.000webhostapp.com/");
	sf::Http::Request request("/" + inpath, sf::Http::Request::Get);
	sf::Http::Response response = http.sendRequest(request);

	if (response.getStatus() == response.NotFound)
	{
		if (!silent)
			std::cout << "file does not exist on remote server (404)" << std::endl;

		return sf::Http::Response::NotFound;
	}
	else if (response.getStatus() == response.InternalServerError)
	{
		if (!silent)
			std::cout << "internal server error was encountered, aborting..." << std::endl;

		return sf::Http::Response::InternalServerError;
	}

	fileSize = response.getBody().size();
	if (!silent)
		std::cout << "downloading remote (" << fileSize << "b (" << getAppropriateFileSize(fileSize, 2) << "))... ";
	 
	fileBuffer = response.getBody();
	if (!silent)
		std::cout << "done downloading." << std::endl;

	return sf::Http::Response::Ok;
}

void Download::save()
{
	std::ofstream downloadFile(outdir + outfile, std::ios::out | std::ios::binary);
	if (!silent)
		std::cout << "saving file to \"" << outdir << outfile << "\"... ";

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
	int bytesPerUnit = 1000;
	std::string sizes[] = { "Bytes", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB", "UNDEFINED" };
	int i = std::floor(std::log(bytes) / std::log(bytesPerUnit));

	std::string number = std::to_string(bytes / std::pow(bytesPerUnit, i));
	number.erase(number.find('.') + decimals + 1, number.length());
	return number + " " + sizes[i];
}

// private:

void Download::createDirectory(std::string dir)
{
	namespace fs = std::experimental::filesystem;

//	std::cout << "creating " << dir << "... ";

	std::vector<fs::path> subdirectories;
	dir.erase(0, 2); // .\\

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

//	std::cout << "done." << std::endl;
}

bool Download::exists(std::string thing)
{
//	std::cout << "checking if " << thing << " exists... ";

	if (std::experimental::filesystem::exists(thing))
	{
//		std::cout << "yes" << std::endl;

		return true;
	}
	else
	{
//		std::cout << "no" << std::endl;

		return false;
	}
}
