#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <SFML/Graphics/Color.hpp>
#include <string>

class ResourceManager;

// TODO: redo this system

namespace GBL
{
	const int VERSION = 16;

	namespace DIR
	{
//		const std::string installDir = "";
//		const std::string resources	 = installDir + "/bin/resources/";
//		const std::string textures	 = resources + "textures/";
//		const std::string fonts		 = resources + "fonts/";
//		const std::string themes	 = resources + "themes/";
//		const std::string stats		 = installDir + "stats/";

		const std::string BASE	   = "/bin/";
		const std::string CACHE	   = "/cache/";
		const std::string RESOURCE = "/resources/";
		const std::string THEME	   = "/themes/";
		const std::string TEXTURE  = "/textures/";
		const std::string FONT	   = "/fonts/";
		const std::string APPS	   = "/apps/";
		const std::string STATS    = "/stats/";
		const std::string CONFIG   = BASE + "/kunlauncher.conf";
	}

	namespace WEB
	{
		const std::string HOSTNAME	= "kunlauncher.000webhostapp.com/";
		const std::string APPS		= "apps/";
		const std::string VERSION	= "launcher/version/";

		namespace LATEST
		{
			const std::string DIR			= "launcher/version/latest/";
			const std::string RESOURCE		= "launcher/version/latest/resources/";
			const std::string RESOURCE_LIST = "launcher/version/latest/resources/resources.dat";
			const std::string EXECUTABLE	= DIR + "latest.noexe";
		}
	}

	namespace COLOR
	{
		extern sf::Color BACKGROUND;
		extern sf::Color TEXT;

		namespace SCROLLBAR
		{
			extern sf::Color SCROLLBAR;
			extern sf::Color SCROLLTHUMB;
			extern sf::Color SCROLLTHUMB_HOVER;
			extern sf::Color SCROLLTHUMB_HOLD;
		}

		namespace ITEM
		{
			extern sf::Color CARD;
			extern sf::Color ICON;
			extern sf::Color REDOWLOAD;
			extern sf::Color UPDATE_IS_AVAILABLE;
			extern sf::Color DOWNLOAD;
		}

		namespace LINK
		{
			extern sf::Color CARD;
			extern sf::Color FOLLOW;
			extern sf::Color TEXT;
		}
	}
}

#endif // !GLOBALS_HPP
