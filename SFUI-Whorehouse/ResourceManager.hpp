#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <string>

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	void loadTexture(std::string resourceName, std::string fileLocation);
	void freeTexture(std::string filename);
	void freeAllTextures();
	bool isTextureLoaded(std::string filename);
	sf::Texture* getTexture(std::string filename);

	void freeAll();

private:
	std::map<std::string, sf::Texture*> loadedTextures;
};

#endif
