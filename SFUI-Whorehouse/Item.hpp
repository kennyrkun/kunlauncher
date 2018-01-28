#ifndef ITEM_HPP
#define ITEM_HPP

#include <SFML/Graphics.hpp>
#include <SFUI/TextButton.hpp>

class Item
{
public:
	//Item(std::string itemName_, sf::RenderWindow* target_window, float xPos);
	Item(std::string itemName_, sf::RenderWindow* target_window, float xSize, float ySize, float xPos, float yPos);
	~Item();

	sf::RectangleShape cardShape;
	sf::RectangleShape downloadButton;
	sf::CircleShape    redownloadButton;
	sf::RectangleShape removeButton;
	sf::RectangleShape launchButton;
//	sf::RectangleShape controlBar;

	sf::Text	name;
	std::string itemName;
	float totalHeight;

	int cardNumber; // used to store depth
	bool downloaded;
	bool isDownloading;
//	bool isRunning();
	bool updateIsAvailable;

	void deleteFiles();
	bool checkForUpdate();
	void updateItem();
	void download();
	void openItem();

	void updateSize(float xSize, float ySize, float xPos, float yPos);
	void draw();

private:
	sf::RenderWindow* targetWindow;

	sf::Texture		   iconTexture;
	sf::RectangleShape icon;

	sf::Texture downloadButtonTexture;
	sf::Texture redownloadButtonTexture;
	sf::Texture removeButtonTexture;
	sf::Texture launchButtonTexture;

	std::string itemInstallDir;
	sf::Font	font;
	sf::Text	description;
	sf::Text	version;

	std::ifstream& GotoLine(std::ifstream& file, unsigned int line);

	void parseInfo(std::string dir);

	int downloadIcon();
	int downloadInfo();
	int downloadFiles();

	int downloadFile(std::string fileName, std::string inPath, std::string outPath);
	int deleteFile(std::string fileName, std::string filePath);
};

#endif
