#ifndef Globals_HPP
#define Globals_HPP

#include <SFML\Graphics\Color.hpp>
#include <string>

class ResourceManager;

// TODO: redo this system

namespace GBL
{
	const std::string VERSION = "0.14.4";

	namespace DIR
	{
		const std::string BASE	   = "\\bin\\";
		const std::string CACHE	   = "\\cache\\";
		const std::string RESOURCE = "\\resources\\";
		const std::string TEXTURE  = "\\textures\\";
		const std::string FONT	   = "\\fonts\\";
		const std::string APPS	   = "\\apps\\";
		const std::string CONFIG   = BASE + "\\launcherconfig.cfg";

		const std::string WEB_HOSTNAME		= "kunlauncher.000webhostapp.com/";
		const std::string WEB_DIRECTORY		= "./";
		const std::string WEB_APP_DIRECTORY = "/apps/";
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

#endif
