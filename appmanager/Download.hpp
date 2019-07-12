#ifndef DOWNLOAD_HPP
#define DOWNLOAD_HPP

#include "SFML\Network.hpp"

#include <string>

// TODO: replace setOutputFilename and setOutputDir with setOutput
// TODO: add "localserver" thing, instead of goign to kunlauncher.com go to a specific file path which emulates webserver

class Download
{
public:
	enum Status
	{
		Ok,
		Fail,
		LocalFileNotFound,
		RemoteFileNotFound,
		ConnectionFailed,
		ConnectionTimedOut,
		LoginFailed,
		SaveFailed
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

	// TODO: do we want this here? or elswhere
	// sf::Ftp ftp;

	int htmlReturnCode;
	std::string fileBuffer;
	uintmax_t fileSize;
	bool sizeVerified;
	bool downloaded;

private:
	std::string input;

	std::string saveDir = "";
	std::string saveFile = "";

	std::string remoteDirectory = "";
	std::string remoteFilename = "";

	void createDirectory(std::string dir);
};

#endif
