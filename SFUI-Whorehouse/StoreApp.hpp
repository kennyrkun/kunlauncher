#ifndef STORE_APP_HPP
#define STORE_APP_HPP

#include "App.hpp"
#include "AppWindow.hpp"

#include <SFML/Graphics.hpp>

class StoreApp : public App
{
public:
	StoreApp(int appid, float xSize, float ySize, float xPos, float yPos);
	~StoreApp();

	sf::RectangleShape cardShape;

	sf::CircleShape    infoButton;
	sf::RectangleShape openInMyAppsListButton;

	VisualItemInfo infoPanel;

	void setPosition(const sf::Vector2f& pos);
	sf::Vector2f getPosition() { return cardShape.getPosition(); }

	sf::FloatRect getLocalBounds() { return cardShape.getLocalBounds(); }

	int onClick(sf::Vector2f clickPos);

	bool checkForUpdate(sf::Ftp& ftp);

	bool deleteFilesPrompt();
	void deleteFiles();
	void download();
	void openItem();

	// TODO: void openItemInMyAppsList();

	void updateSizeAndPosition(float xSize, float ySize, float xPos, float yPos);

	void update() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Texture		   iconTexture;
	sf::RectangleShape icon;

	sf::Texture openInMyAppsListTexture;

	std::string itemCacheDir;
	std::string itemInstallDir;
	sf::Text	name;

	void parseInfo(std::string dir);

	bool downloadIcon();
	bool downloadInfo();
	bool downloadFiles();

	int install();
};

#endif // !STORE_APP_HPP
