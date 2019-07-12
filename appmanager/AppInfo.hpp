#ifndef APP_INFO_HPP
#define APP_INFO_HPP

#include <SFML/Graphics.hpp>

struct AppInfo
{
	std::string name;
	std::string description;
	std::string version;
	std::string author;
	std::string github;
	int release;
	int appid;
};

#endif // !APP_INFO_HPP
