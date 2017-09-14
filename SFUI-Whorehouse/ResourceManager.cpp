#include <SFML/Graphics.hpp>
#include <map>
#include <string>
#include <iostream>

#include "ResourceManager.hpp"

ResourceManager::ResourceManager()
{
	std::cout << "ResourceManager created." << "\n";
}

ResourceManager::~ResourceManager()
{
	freeAllTextures();

	std::cout << "ResourceManager deconstructed." << "\n";
}

void ResourceManager::loadTexture(std::string resourceName, std::string fileLocation)
{
	if (!isTextureLoaded(resourceName))
	{
		sf::Texture* new_tex = new sf::Texture;
		new_tex->loadFromFile(fileLocation);
		loadedTextures[resourceName] = new_tex;

		std::cout << "Loaded texture\"" + resourceName + "\" from \"" + fileLocation + "\"" << "\n";
	}
}

void ResourceManager::freeTexture(std::string filename)
{
	delete loadedTextures[filename];
	loadedTextures[filename] = nullptr;

	std::cout << "Freed texture \"" + filename + "\"" << "\n";
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

	std::cout << "Freed all textures." << "\n";
}

bool ResourceManager::isTextureLoaded(std::string filename)
{
	// how does this even what

	if (loadedTextures.find(filename) == loadedTextures.end())
		return false;
	else
		return true;
}

sf::Texture* ResourceManager::getTexture(std::string filename)
{
//	loadTexture(filename); // no
	return loadedTextures[filename];
}

void ResourceManager::freeAll()
{
	//as more things are added, remove them one by one.

	// Iterate through all the loaded textures, free them, and delete the pointers
	std::map<std::string, sf::Texture*>::iterator it;

	for (it = loadedTextures.begin(); it != loadedTextures.end(); it++)
	{
		delete it->second;
		it->second = nullptr;
	}

	std::cout << "Freed all textures." << "\n";
}
