#include "ResourceManager.hpp"

#include <map>
#include <string>
#include <iostream>

ResourceManager::ResourceManager()
{
	std::cout << "ResourceManager created." << std::endl;
}

ResourceManager::~ResourceManager()
{
	freeAll();

	std::cout << "ResourceManager deconstructed." << std::endl;
}

void ResourceManager::loadTexture(std::string resourceName, std::string fileLocation)
{
	if (!isTextureLoaded(resourceName))
	{
		sf::Texture* new_tex = new sf::Texture;
		new_tex->loadFromFile(fileLocation);
		loadedTextures[resourceName] = new_tex;

		std::cout << "Loaded texture\"" + resourceName + "\" from \"" + fileLocation + "\"" << std::endl;
	}
}

void ResourceManager::freeTexture(std::string resourceName)
{
	delete loadedTextures[resourceName];
	loadedTextures[resourceName] = nullptr;

	std::cout << "Freed texture \"" + resourceName + "\"" << std::endl;
}

void ResourceManager::freeAllTextures()
{
	// Iterate through all the loaded textures, free them, and delete the pointers
	std::map<std::string, sf::Texture*>::iterator it;

	for (it = loadedTextures.begin(); it != loadedTextures.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}

	std::cout << "Freed all textures." << std::endl;
}

bool ResourceManager::isTextureLoaded(std::string resourceName)
{
	// how does this even what

	if (loadedTextures.find(resourceName) == loadedTextures.end())
		return false;
	else
		return true;
}

sf::Texture* ResourceManager::getTexture(std::string resourceName)
{
//	loadTexture(filename); // no
	return loadedTextures[resourceName];
}

void ResourceManager::freeAll()
{
	//as more things are added, remove them one by one.

	freeAllTextures();

	std::cout << "Freed all resources." << std::endl;
}
