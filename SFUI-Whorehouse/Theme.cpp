#include "Theme.hpp"

#include "Globals.hpp"
#include "SettingsParser.hpp"
#include "MessageBox.hpp"

#include <fstream>
#include <iostream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

bool Theme::loadFromFile(std::string file)
{
	// load all colours
	// interate over all files in the resources folder and check if they're being overriden

	SettingsParser themeParser;
	if (themeParser.loadFromFile(GBL::DIR::themes + file + "/" + file + ".sfuitheme"))
	{
		themeParser.get("name", name);
		themeParser.get("author", author);
		themeParser.get("version", version);
		// TODO: rename this to launcherVersion
		themeParser.get("launcherversion", themeLauncherVersion);

		{
			std::vector<int> colors; // TODO: improve this conversion

			if (themeParser.get("primary", colors))
				palatte.PRIMARY = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("secondary", colors))
				palatte.SECONDARY = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("tertiary", colors))
				palatte.TERTIARY = sf::Color(colors[0], colors[1], colors[2]);


			if (themeParser.get("text", colors))
				palatte.TEXT_DARK = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("text_secondary", colors))
				palatte.TEXT_LIGHT = sf::Color(colors[0], colors[1], colors[2]);


			if (themeParser.get("scrollbar", colors))
				palatte.SCROLLBAR = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("scrollthumb", colors))
				palatte.SCROLLTHUMB = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("scrollthumb_hover", colors))
				palatte.SCROLLTHUMB_HOVER = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("scrollthumb_hold", colors))
				palatte.SCROLLTHUMB_HOLD = sf::Color(colors[0], colors[1], colors[2]);


			if (themeParser.get("app_card", colors))
				palatte.APP_CARD = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_card2", colors))
				palatte.APP_CARD2 = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_image", colors))
				palatte.APP_IMAGE = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon", colors))
				palatte.APP_ICON = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_hover", colors))
				palatte.APP_ICON_HOVER = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_press", colors))
				palatte.APP_ICON_PRESS = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_fail", colors))
				palatte.APP_ICON_FAIL = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_fail_hover", colors))
				palatte.APP_ICON_FAIL_HOVER = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_fail_press", colors))
				palatte.APP_ICON_FAIL_PRESS = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_warn", colors))
				palatte.APP_ICON_WARN = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_warn_hover", colors))
				palatte.APP_ICON_WARN_HOVER = sf::Color(colors[0], colors[1], colors[2]);

			if (themeParser.get("app_icon_warn_press", colors))
				palatte.APP_ICON_WARN_PRESS = sf::Color(colors[0], colors[1], colors[2]);
		}
		
		// TODO: improve this

		// Check for any overriden files
		SettingsParser getResources;
		if (getResources.loadFromFile(GBL::DIR::resources + "resources.dat") && 
			themeParser.loadFromFile(GBL::DIR::themes + name + "/" + name + ".sfuitheme"))
		{
			std::vector<std::string> textureList;
			std::vector<std::string> fontList;

			getResources.get("textures", textureList);
			getResources.get("fonts", fontList);

			// if files 
			bool filesAreOverriden(false);
			if (themeParser.get("overriden_files", overriden_files))
				filesAreOverriden = true;

			for (size_t i = 0; i < textureList.size(); i++)
			{
				// if the texture exists, load it and put it into the textures map
				if (fs::exists(GBL::DIR::textures + textureList[i]))
				{
					try
					{
						sf::Texture* tex = new sf::Texture;
						tex->loadFromFile(GBL::DIR::textures + textureList[i]);
						tex->setSmooth(true);
						textures[textureList[i]] = tex;
					}
					catch (const std::exception& e)
					{
						std::cerr << e.what() << std::endl;

						MessageBox::Options modOptions;
						modOptions.title = "Error";
						modOptions.text = e.what();
						modOptions.settings = { "Ok"};

						MessageBox failedToSave(modOptions);
						failedToSave.runBlocking();
					}
				}
				else
					std::cerr << fontList[i] << " was specified, but does not exist" << std::endl;

				// if the texture is reported as overriden
				if (filesAreOverriden && std::find(overriden_files.begin(), overriden_files.end(), textureList[i]) != overriden_files.end())
				{
					// if the overriden texture exists
					if (fs::exists(GBL::DIR::themes + name + "/" + textureList[i]))
					{
						std::cout << "overriding \"" + textureList[i] + "\"" << std::endl;
						
						// load it into the overriden textures map
						sf::Texture* tex = new sf::Texture;
						tex->loadFromFile(GBL::DIR::themes + name + "/" + textureList[i]);
						overriden_textures[textureList[i]] = tex;
					}
					else
						std::cerr << textureList[i] << " is overriden, but a replacement is not provided!" << std::endl;
				}
			}

			for (size_t i = 0; i < fontList.size(); i++)
			{
				// if the texture exists, load it and put it into the textures map
				if (fs::exists(GBL::DIR::fonts + fontList[i]))
				{
					sf::Font* font = new sf::Font;
					font->loadFromFile(GBL::DIR::fonts + fontList[i]);
					fonts[fontList[i]] = font;
				}
				else
					std::cerr << fontList[i] << " was specified, but does not exist" << std::endl;

				// if the texture is reported as overriden
				if (filesAreOverriden && std::find(overriden_files.begin(), overriden_files.end(), fontList[i]) != overriden_files.end())
				{
					// if the overriden texture exists
					if (fs::exists(GBL::DIR::themes + name + "/" + fontList[i]))
					{
						std::cout << "overriding \"" + fontList[i] + "\"" << std::endl;

						// load it into the overriden fonts map
						sf::Font* font = new sf::Font;
						font->loadFromFile(GBL::DIR::themes + name + "/" + fontList[i]);
						overriden_fonts[fontList[i]] = font;
					}
					else
						std::cerr << fontList[i] << " is overriden, but a replacement is not provided!" << std::endl;
				}
			}
		}
		else
		{
			std::cerr << "failed to load resources" << std::endl;
			return false;
		}

		if (isResourceOverriden("interface_square.png"))
			SFUI::Theme::loadTexture(getTexture("interface_square.png"));

		SFUI::Theme::click.textColor = palatte.TEXT_DARK;
		SFUI::Theme::click.textColorHover = palatte.TEXT_DARK;
		SFUI::Theme::click.textColorFocus = palatte.TEXT_DARK;
		SFUI::Theme::input.textColor = palatte.TEXT_DARK;
		SFUI::Theme::input.textColorHover = palatte.TEXT_DARK;
		SFUI::Theme::input.textColorFocus = palatte.TEXT_DARK;
	}
	else
	{
		std::cerr << "failed to load theme file" << std::endl;
		return false;
	}

	return true;
}

// not part of Theme
std::string colorAsString(const sf::Color& color)
{
	return std::to_string(color.r) + ", " + std::to_string(color.g) + ", " + std::to_string(color.b);
}

bool Theme::saveToFile(std::string file)
{
	std::string s(file.size() ? file : name);

	SettingsParser themeParser;
	if (themeParser.loadFromFile(GBL::DIR::themes + s + "/" + s + ".sfuitheme"))
	{
		themeParser.set("name", name);
		themeParser.set("author", author);
		themeParser.set("version", version);
		themeParser.set("launcherversion", themeLauncherVersion);

		themeParser.set("primary", colorAsString(palatte.PRIMARY));
		themeParser.set("secondary", colorAsString(palatte.SECONDARY));
		themeParser.set("tertiary", colorAsString(palatte.TERTIARY));

		themeParser.set("text", colorAsString(palatte.TEXT_DARK));
		themeParser.set("text_secondary", colorAsString(palatte.TEXT_LIGHT));

		themeParser.set("scrollbar", colorAsString(palatte.SCROLLBAR));
		themeParser.set("scrollthumb", colorAsString(palatte.SCROLLTHUMB));
		themeParser.set("scrollthumb_hover", colorAsString(palatte.SCROLLTHUMB_HOVER));
		themeParser.set("scrollthumb_hold", colorAsString(palatte.SCROLLTHUMB_HOLD));

		themeParser.set("app_card", colorAsString(palatte.APP_CARD));
		themeParser.set("app_card2", colorAsString(palatte.APP_CARD2));
		themeParser.set("app_image", colorAsString(palatte.APP_IMAGE));
		themeParser.set("app_icon", colorAsString(palatte.APP_ICON));
		themeParser.set("app_icon_hover", colorAsString(palatte.APP_ICON_HOVER));
		themeParser.set("app_icon_press", colorAsString(palatte.APP_ICON_PRESS));
		themeParser.set("app_icon_fail", colorAsString(palatte.APP_ICON_FAIL));
		themeParser.set("app_icon_fail_hover", colorAsString(palatte.APP_ICON_FAIL_HOVER));
		themeParser.set("app_icon_fail_press", colorAsString(palatte.APP_ICON_FAIL_PRESS));
		themeParser.set("app_icon_warn", colorAsString(palatte.APP_ICON_WARN));
		themeParser.set("app_icon_warn_hover", colorAsString(palatte.APP_ICON_WARN_PRESS));
		themeParser.set("app_icon_warn_press", colorAsString(palatte.APP_ICON_WARN_PRESS));

		themeParser.set("overriden_files", overriden_files);
	}
	else
	{
		std::cerr << "failed to open theme file for saving" << std::endl;
		return false;
	}

	std::cout << "Save successfully" << std::endl;
	return true;
}

bool Theme::isResourceOverriden(std::string resource)
{
	for (size_t i = 0; i < overriden_textures.size(); i++)
		// if the file exists in the map, it's been overwritten
		if (overriden_textures.find(resource) == overriden_textures.end())
			return false;

	for (size_t i = 0; i < overriden_fonts.size(); i++)
		if (overriden_textures.find(resource) == overriden_textures.end())
			return false;

	return true;
}

sf::Font* Theme::getFont(std::string font, bool original)
{
	try
	{
		if (!original && overriden_fonts.find(font) != overriden_fonts.end())
		{
			dout("FONT is overriden");
			return overriden_fonts[font];
		}

		if (fonts.find(font) == fonts.end())
		{
			derr("FONT is bad (nullptr)");
			return nullptr;
		}
		else
		{
			dout("FONT is good");
			return fonts[font];
		}
	}
	catch (const std::exception&e)
	{
		std::cerr << "[THEME] FONT: Exception:" << std::endl;
		std::cerr << e.what() << std::endl;

		return nullptr;
	}
}

sf::Texture* Theme::getTexture(std::string texture, bool original)
{
	try
	{
		if (!original && overriden_textures.find(texture) != overriden_textures.end())
		{
			dout("TEXTURE is overriden");
			return overriden_textures[texture];
		}

		if (textures.find(texture) == textures.end())
		{
			derr("TEXTURE is bad (nullptr)");
			return nullptr;
		}
		else
		{
			dout("TEXTURE is good");
			return textures[texture];
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "[THEME] TEXTURE: Exception:" << std::endl;
		std::cerr << e.what() << std::endl;

		return nullptr;
	}
}

bool Theme::removeOverridenFile(std::string file)
{
	if (isResourceOverriden(file))
	{
		std::string ext = file.substr(file.rfind("."), file.size());
		if (ext == ".png")
			overriden_textures.erase(file);
		else if (ext == ".ttf")
			overriden_fonts.erase(file);

		overriden_files.erase(std::remove(overriden_files.begin(), overriden_files.end(), file), overriden_files.end());

		return true;
	}

	return false;
}

bool Theme::addOverridenFile(std::string file)
{
	std::cout << "attempting to override file: " << file << std::endl;

	std::string ext("NO_EXT");
	const int texture = 1;
	const int font = 2;
	const int unknown = 3;
	int type;

	if (file.find(".") != std::string::npos)
	{
		ext = file.substr(file.rfind(".", file.size())); // last file type in the file
		// TODO: support more filetypes
		if (ext == ".png")
			type = texture;
		else if (ext == ".ttf")
			type = font;
		else // unknown
			type = unknown;
	}
	else
		type = unknown;

	if (fs::exists(GBL::DIR::themes + name + "/" + file))
	{
		switch (type)
		{
		case texture:
		{
			sf::Texture* tex = new sf::Texture;
			tex->loadFromFile(GBL::DIR::themes + name + "/" + file);
			overriden_textures[file] = tex;
			return true;
			break;
		}
		case font:
		{
			sf::Font* font = new sf::Font;
			font->loadFromFile(GBL::DIR::themes + name + "/" + file);
			overriden_fonts[file] = font;
			return true;
			break;
		}
		case unknown:
			std::cerr << "unknown resource type (" << ext << ")" << std::endl;
			break;
		}
	}
	else
	{
		std::cerr << "file does not exist" << std::endl;
	}

	std::cerr << "failed to override file" << std::endl;
	return false;
}

void Theme::updateOverridenFiles(std::string files)
{
	std::vector<std::string> newValues; {
		std::istringstream iss(files);
		std::string output;

		while (std::getline(iss, output, ','))
			newValues.push_back(output);
	}

	std::vector<std::string> oldValues; {
		std::istringstream iss(overridenFilesAsString());
		std::string output;

		while (std::getline(iss, output, ','))
			oldValues.push_back(output);
	}

	std::cout << "[THEME] old: " << overridenFilesAsString() << std::endl;
	std::cout << "[THEME] new: " << files << std::endl;

	for (size_t i = 0; i < oldValues.size(); i++)
		if (std::find(newValues.begin(), newValues.end(), oldValues[i]) == newValues.end())
		{
			std::cout << "[THEME] old value \"" << oldValues[i] << "\" is no longer overriden!" << std::endl;
			removeOverridenFile(oldValues[i]);
		}

	for (size_t i = 0; i < newValues.size(); i++)
	{
		std::string ext;
		if (newValues[i].find('.') != std::string::npos)
			ext = newValues[i].substr(newValues[i].rfind(".", newValues[i].size())); // last file type in the file
		else
			ext = "NO_EXT";
		// TODO: if the string doesn't have an extension, remove it

		bool succ = false;
		if (ext == ".png")
			if (getTexture(newValues[i]) != nullptr) // if it actually exists
				if (!isResourceOverriden(newValues[i])) // make sure it's not already overriden
				{
					if (addOverridenFile(newValues[i])) // make sure the file actually got added
						succ = true; // we are succ
				}
				else // reload the texture
				{
					sf::Texture* tex = getTexture(newValues[i]);
					tex->loadFromFile(GBL::DIR::themes + name + "/" + newValues[i]);
				}
		else if (ext == ".ttf") // true type font
			if (getFont(newValues[i]) != nullptr)
				if (!isResourceOverriden(newValues[i]))
				{
					if (addOverridenFile(newValues[i]))
						succ = true;
				}
				else
				{
					sf::Font* fn = getFont(newValues[i]);
					fn->loadFromFile(GBL::DIR::themes + name + "/" + newValues[i]);
				}

		if (succ)
			overriden_files.push_back(newValues[i]);
	}

	std::cout << "new: " << overridenFilesAsString() << std::endl;
}
