#ifndef DOWNLOAD_HPP
#define DOWNLOAD_HPP

#include "SFML\Network.hpp"

#include <string>

// TODO: exists(s): check if folder exists
// this will be used to make sure apps exist and aren't just in the index
// TODO: replace setOutputFilename and setOutputDir with setOutput
// TODO: add "localserver" thing, instead of goign to kunlauncher.com go to a specific file path which emulates webserver

class Download
{
public:
	enum Status
	{
		Fail,
		Success,

		ConnectionFailed,
		ConnectionTimedOut,
		LoginFailed,
	};

public:
	Download();
	~Download();

	void setOutputDir(std::string directory);
	std::string getOutputDir();

	void setOutputFilename(std::string filename);
	std::string getOutputFilename();

	void setInput(std::string pathandfile);
	std::string getInput();

	void setInputDirectory(std::string directory);
	std::string getInputDirectory();

	void setInputFilename(std::string filename);
	std::string getInputFilename();

	uintmax_t getFileSize();
	int download();
	int save();

	std::string getAppropriateFileSize(const long long int bytes, const int decimals);

	void clearCache();
	void cleanup();

	// TODO: resusable FTP object in class, or single use per function
	// sf::Ftp ftp;

	// TODO: download objects into a void* data
	// don't save to cache until necessary

	int htmlReturnCode;
	std::string fileBuffer;
	uintmax_t fileSize;
	bool downloaded;
	// this is used to make sure that all the file's contents were downloaded
	// it is not perfect, but it's much better than none.
	bool sizeVerified;

private:
	std::string input;

	std::string saveDir = ".//bin//cache";
	std::string saveFile = "";

	std::string remoteDirectory = "";
	std::string remoteFilename = "";

	sf::Ftp ftp;

	int setupFTP();

	void createDirectory(std::string dir);
};

#endif
