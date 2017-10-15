#include "AppSettings.hpp"

#include "Globals.hpp"

void AppSettings::getSettings()
{
	std::ifstream getSettings(CONST::DIR::CONFIG);
	std::string rawSettings;
	getSettings >> rawSettings;

	
	// parse
	// store
}

void AppSettings::createDefaultSettings()
{
	std::ofstream create(CONST::DIR::CONFIG, std::ios::out | std::ios::binary);
	create.close();
}

std::ifstream& AppSettings::GotoLine(std::ifstream& file, unsigned int line)
{
	file.seekg(std::ios::beg); // go to top

	for (size_t i = 0; i < line - 1; ++i) // go to line
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	return file; // return line
}
