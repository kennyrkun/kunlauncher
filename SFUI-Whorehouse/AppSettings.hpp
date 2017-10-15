#ifndef APP_SETTINGS_HPP
#define APP_SETTINGS_HPP

#include <fstream>

class AppSettings
{
public:
#ifdef _DEBUG
	bool updateLauncherOnStart = false;
	bool updateItemIndexOnStart = false;
#else
	bool updateLauncherOnStart = true;
	bool updateItemsOnStart = true;
#endif
	bool fullscreen = false;
	bool verticalSync = true;

	int width = 525;
	int height = 375;

	void getSettings();

private:
	void createDefaultSettings();

	std::ifstream& GotoLine(std::ifstream& file, unsigned int line);
};

#endif