#ifndef APP_INFO_HPP
#define APP_INFO_HPP

#include "../SFUI-Whorehouse/SettingsParser.hpp"
#include "Globals.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>

struct AppInfo
{
	std::string name;
	std::string description;
	std::string version;
	std::string author;
	std::string github;

	std::string datapath;
	std::string iconpath;

	int release;

	// TODO:
	// typedef uint64_t appid_t;
	int appid;

	bool loadByAppID(int id)
	{
		SettingsParser parser;

		if (parser.loadFromFile(GBL::DIR::apps + std::to_string(id) + "/info.dat"))
		{
			parser.get("name", name);
			parser.get("description", description);
			parser.get("version", version);
			parser.get("author", author);
			parser.get("github", github);
			parser.get("release", release);
			parser.get("appid", appid);

			return true;
		}
		else
		{
			std::cerr << "failed to load app info at " << id << std::endl;
			return false;
		}
	}
};

#endif // !APP_INFO_HPP
