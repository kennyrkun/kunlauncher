#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <SFML\Graphics\Color.hpp>
#include <string>

namespace zzz
{
	const std::string LAUNCHER_DIRECTORY     	= "C:\\Program Files (x86)\\KunLauncher\\";
	const std::string LAUNCHER_ROOT_DIRECTORY	= ".\\";
	const std::string LAUNCHER_CONFIG			= "kunlauncher.cfg";
	const std::string CACHE_DIRECTORY			= "cache";

	const std::string REMOTE_HOSTNAME  = "kunlauncher.000webhostapp.com";
	const std::string REMOTE_ROOT_DIRECTORY = "./";

	const std::string APP_DIRECTORY		 = "kunapps";
	const std::string RESOURCE_DIRECTORY = "resources";
	const std::string TEXTURE_DIRECTORY  = "textures";
	const std::string FONT_DIRECTORY	 = "fonts";
}

const std::string LAUNCHER_CONFIG = "kunlauncher.cfg";

const std::string WEB_HOSTNAME = "kunlauncher.000webhostapp.com";
const std::string WEB_DIRETORY = "./";

const std::string BASE_DIRECTORY		= "bin";
const std::string BASE_BRANCH_DIRECTORY = "master";
const std::string CACHE_DIRECTORY		= "cache";
const std::string RESOURCE_DIRECTORY	= "resources";
const std::string TEXTURE_DIRECTORY		= "textures";
const std::string FONT_DIRECTORY		= "fonts";

const std::string APPS	   = "apps";
const std::string APP_ICON = "icon.png";
const std::string APP_DATA = "appdata.dat";

namespace CONST_COLOURS
{
	const sf::Color BACKGROUND = sf::Color(50, 50, 50);

	namespace SCROLLBAR
	{
		const sf::Color SCROLLBAR		  = sf::Color(80, 80, 80);
		const sf::Color SCROLLTHUMB		  = sf::Color(110, 110, 110);
		const sf::Color SCROLLTHUMB_HOVER = sf::Color(239, 235, 239);
	}

	namespace ITEM
	{
		const sf::Color CARD		= sf::Color(100, 100, 100);
		const sf::Color ICON		= sf::Color(255, 255, 255);
		const sf::Color ICON_HOVER  = sf::Color(255, 255, 255);
		const sf::Color REDOWLOAD	= sf::Color(255, 255, 255);
		const sf::Color DOWNLOAD	= sf::Color(255, 255, 255);
	}
	
	namespace LINK
	{
		const sf::Color LINK_CARD	= sf::Color(255, 255, 255);
		const sf::Color LINK_FOLLOW	= sf::Color(255, 255, 255);
	}
}

#endif
