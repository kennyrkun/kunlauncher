#ifndef DOWNLOAD_HPP
#define DOWNLOAD_HPP

#include <string>

class Download
{
public:
	enum Status
	{
		Ok,
		Failure,
		LocalFileNotFound,
		RemoteFileNotFound,
		ConnectionFailed,
		ConnectionTimedOut,
		ConnectionRejected,
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

	int htmlReturnCode;
	std::string fileBuffer;
	uintmax_t fileSize;
	bool downloaded;

private:
	std::string input;

	std::string saveDir = ".//bin//cache";
	std::string saveFile = "";

	std::string remoteDirectory = "";
	std::string remoteFilename = "";

	void createDirectory(std::string dir);
};

#endif
