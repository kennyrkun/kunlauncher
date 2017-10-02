#ifndef DOWNLOAD_HPP
#define DOWNLOAD_HPP

#include <string>

class Download
{
public:
	Download(bool);
	Download();
	~Download();

	void setOutputDir(std::string dir);
	std::string getOutputDir();

	void setOutputFilename(std::string file);
	std::string getOutputFilename();

	void setInputPath(std::string path);
	std::string getInputPath();

	int download();
	void save();

	std::string getAppropriateFileSize(const long long int bytes, const int decimals);

	int htmlReturnCode;
	std::string fileBuffer;
	uintmax_t fileSize;

private:
	std::string outdir = ".\\";
	std::string outfile = "file.file";
	std::string inpath = "./";

	bool silent = false;

	void createDirectory(std::string dir);
};

#endif
