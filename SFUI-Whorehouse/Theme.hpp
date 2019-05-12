#ifndef THEME_HPP
#define THEME_HPP

#include "Version.hpp"

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

// TODO: make this static

class Theme
{
public:
	std::string name;
	std::string author;
	float version;
	// TODO: use an actual GBL::Version
	// then just use .asString to show things
	// TODO: alert the user when the theme they have is outdated
	// version of the launcher the theme was built

	// TODO: rename this to ThemeAPIVersion
	std::string themeLauncherVersion;

	// file: the name of the theme to be loaded
	// this assumes the theme file is in GBL::DIR::themes, and has the correct file extension
	bool loadFromFile(std::string file);

	// file: the name of the theme to be loaded
	// this assumes the theme file is in GBL::DIR::themes, and has the correct file extension
	// if file is not provided, it will use the file name
	bool saveToFile(std::string file = "");

	// checks if a certain named resource is overriden
	// resource: checks only for filename, will not work with path.
	//			 file extension must be included.
	bool isResourceOverriden(std::string resource);

	// returns a font from resources
	// original: if true, returns original font.
	//			if false, and the font is overriden
	//			the overriden font will be returned
	// nullptr: will return nullptr if the texture does not exist
	sf::Font* getFont(std::string font, bool original = false);

	// returns a texture from resources
	// original: if true, returns original texture.
	//			if false, and the texture is overriden
	//			the overriden texture will be returned
	// nullptr: will return nullptr if the texture does not exist
	sf::Texture* getTexture(std::string texture, bool original = false);

	struct ColorPalette
	{
		sf::Color PRIMARY;
		sf::Color SECONDARY;
		sf::Color TERTIARY;

		sf::Color TEXT;
		sf::Color TEXT_SECONDARY;

		sf::Color SCROLLBAR;
		sf::Color SCROLLTHUMB;
		sf::Color SCROLLTHUMB_HOVER;
		sf::Color SCROLLTHUMB_HOLD;

		// TODO: progress bar

		sf::Color APP_CARD;
		sf::Color APP_CARD2;
		sf::Color APP_IMAGE;
		sf::Color APP_ICON;
		sf::Color APP_ICON_HOVER;
		sf::Color APP_ICON_PRESS;
		sf::Color APP_ICON_FAIL;
		sf::Color APP_ICON_FAIL_HOVER;
		sf::Color APP_ICON_FAIL_PRESS;
		sf::Color APP_ICON_WARN;
		sf::Color APP_ICON_WARN_HOVER;
		sf::Color APP_ICON_WARN_PRESS;
	} palatte;

	// returns a concatenated string of all overwritten files
	// as they would be represented in the configuration file.
	// FIXME: this function can be made better
	std::string overridenFilesAsString()
	{
		if (!overriden_files.empty())
		{
			std::string files = overriden_files[0];
			for (size_t i = 1; i < overriden_files.size(); i++)
				files += "," + overriden_files[i];
			return files;
		}
		else
		{
			return "";
		}
	}

	// removes an overriden file from the list
	// returns true if removal was successuful, false otherwise
	bool removeOverridenFile(std::string file);
	// adds an overriden file to the list.
	// returns true if addition was successuful, false otherwise
	bool addOverridenFile(std::string file);

	// will completely replace all overriden files with the list in this string.
	void updateOverridenFiles(std::string files);

private:
	std::map<std::string, sf::Texture*> textures;
	std::map<std::string, sf::Font*> fonts;

	std::map<std::string, sf::Texture*> overriden_textures;
	std::map<std::string, sf::Font*> overriden_fonts;

	std::vector<std::string> overriden_files;
};

#endif // !THEME_HPP