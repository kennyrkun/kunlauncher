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

	void setOutputFile(std::string file);
	std::string getOutputFile();

	void setInputPath(std::string path);
	std::string getInputPath();

	int download();
	void save();

	std::string getAppropriateFileSize(const long long int bytes, const int decimals);

	std::string fileBuffer;
	uintmax_t fileSize;

private:
	std::string outdir;
	std::string outfile;
	std::string inpath;

	bool silent;

	void createDirectory(std::string dir);
	bool exists(std::string thing);
};

#endif
