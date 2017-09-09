#ifndef Globals_HPP
#define Globals_HPP

#include <SFML\Graphics\Color.hpp>
#include <string>

namespace CONST
{
	const std::string VERSION = "0.12.2";

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
		const sf::Color BACKGROUND = sf::Color(50, 50, 50);

		namespace SCROLLBAR
		{
			const sf::Color SCROLLBAR = sf::Color(80, 80, 80);
			const sf::Color SCROLLTHUMB = sf::Color(110, 110, 110);
			const sf::Color SCROLLTHUMB_HOVER = sf::Color(158, 158, 158);
			const sf::Color SCROLLTHUMB_HOLD = sf::Color(239, 235, 239);
		}

		namespace ITEM
		{
			const sf::Color CARD = sf::Color(100, 100, 100);
			const sf::Color ICON = sf::Color(255, 255, 255);
			const sf::Color ICON_HOVER = sf::Color(255, 255, 255);
			const sf::Color REDOWLOAD = sf::Color(255, 255, 255);
			const sf::Color UPDATE_IS_AVAILABLE = sf::Color(255, 200, 0);
			const sf::Color DOWNLOAD = sf::Color(255, 255, 255);
		}

		namespace LINK
		{
			const sf::Color LINK_CARD = sf::Color(255, 255, 255);
			const sf::Color LINK_FOLLOW = sf::Color(255, 255, 255);
		}
	}
}

#endif
