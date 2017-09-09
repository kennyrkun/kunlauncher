#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <iostream>

#include "ResourceManager.hpp"

ResourceManager::ResourceManager()
{
	std::cout << "ResourceManager created." << std::endl;
}

ResourceManager::~ResourceManager()
{
	freeAllTextures();

	std::cout << "ResourceManager deconstructed." << std::endl;
}

void ResourceManager::loadTexture(std::string resourceName, std::string fileLocation)
{
	if (!textureLoaded(resourceName))
	{
		sf::Texture* new_tex = new sf::Texture;
		new_tex->loadFromFile(fileLocation);
		loaded_textures[resourceName] = new_tex;

		std::cout << "Loaded \"" + resourceName + "\" from \"" + fileLocation + "\"" << std::endl;
	}
}

void ResourceManager::freeTexture(std::string filename)
{
	delete loaded_textures[filename];
	loaded_textures[filename] = nullptr;

	std::cout << "Freed texture \"" + filename + "\"" << std::endl;
}

void ResourceManager::freeAllTextures()
{
	// Iterate through all the loaded textures, free them, and delete the pointers
	std::map<std::string, sf::Texture*>::iterator it;

	for (it = loaded_textures.begin(); it != loaded_textures.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}

	std::cout << "Freed all textures." << std::endl;
}

bool ResourceManager::textureLoaded(std::string filename)
{
	// how does this even what

	if (loaded_textures.find(filename) == loaded_textures.end())
		return false;
	else
		return true;
}

sf::Texture* ResourceManager::getTexture(std::string filename)
{
//	loadTexture(filename); // no
	return loaded_textures[filename];
}