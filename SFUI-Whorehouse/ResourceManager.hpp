#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

// TODO: template resource class

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void loadTexture(std::string resourceName, std::string fileLocation);
	void freeTexture(std::string resourceName);
	void freeAllTextures();
	bool isTextureLoaded(std::string resourceName);
	sf::Texture* getTexture(std::string resourceName);

	void freeAll();

private:
	std::map<std::string, sf::Texture*> loadedTextures;
};

#endif
