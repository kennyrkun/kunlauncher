#ifndef STORE_APP_HPP
#define STORE_APP_HPP

#include "App.hpp"
#include "AppWindow.hpp"

#include <SFML/Graphics.hpp>

class StoreApp : public App
{
public:
	StoreApp(int appid, float xSize, float ySize, const sf::Vector2f& position);
	~StoreApp();

	ItemInfo info;

	sf::RectangleShape cardShape;
	sf::CircleShape    infoButton;
	sf::RectangleShape openInMyAppsListButton;

	void setPosition(const sf::Vector2f& pos);
	sf::Vector2f getPosition() { return cardShape.getPosition(); }

	sf::FloatRect getLocalBounds() { return cardShape.getLocalBounds(); }

	int onClick(sf::Vector2f clickPos);

	bool deleteFilesPrompt();
	void deleteFiles();
//	bool checkForUpdate();
//	void updateItem();
	void download();
	void openItem();

	// TODO: void openItemInMyAppsList();

	void updateSizeAndPosition(float xSize, float ySize, float xPos, float yPos);

	void update() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

	VisualItemInfo infoPanel;

private:
	sf::Texture		   iconTexture;
	sf::RectangleShape icon;

	sf::Texture openInMyAppsListTexture;

	std::string itemCacheDir;
	std::string itemInstallDir;
	sf::Text	name;

	void parseInfo(std::string dir);

	// upload the app icon to the appcache folder
	int downloadIcon();
	// upload the info icon to the appcache folder
	int downloadInfo();
	// copy the app's files to the apps folder
	int downloadFiles();
};

#endif // !STORE_APP_HPP
