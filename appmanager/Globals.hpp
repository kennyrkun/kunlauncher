#ifndef GLOBALS_HPP
#define GLOBALS_HPP

// TODO: use path::make_preferred (preferred_separator)
// do it in KunLauncher too

#include <SFML/Graphics/Color.hpp>

#include <string>

namespace GBL
{
	const float VERSION = 1.3f;
	const std::string NAME = "App Manager";

	namespace DIR
	{
		const std::string installDir = "./appmanagerbin/";
		const std::string apps = installDir + "apps/";
		const std::string cache = installDir + "cache/";
		//const std::string appcache = installDir + "appcache/";
		const std::string resources = installDir + "resources/";
		const std::string textures = resources + "textures/";
		const std::string fonts = resources + "fonts/";
		const std::string themes = resources + "themes/";

		const std::string config = installDir + "appmanager.conf";
	}

	namespace WEB
	{
		// TODO: split public_html somewhere else
		const std::string HOSTNAME = "kunlauncher.000webhostapp.com/public_html/";
		const std::string APPS = "apps/";
		const std::string NEWS = "news/";
		const std::string VERSIONS = "version/";

		namespace LATEST
		{
			const std::string DIR = VERSIONS + "latest/";
			const std::string CHANGES = DIR + "change.log";
		}
	}

	namespace color
	{
		const sf::Color SCROLLBAR = sf::Color(80, 80, 80);
		const sf::Color SCROLLTHUMB = sf::Color(110, 100, 100);
		const sf::Color SCROLLTHUMB_HOVER = sf::Color(158, 158, 158);
		const sf::Color SCROLLTHUMB_HOLD = sf::Color(239, 235, 239);
	}
}

#endif // !GLOBALS_HPP