#ifndef MY_APP_HPP
#define MY_APP_HPP

#include "App.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

class MyApp : public App
{
public:
	MyApp(int appid, float xSize, float ySize, const sf::Vector2f& position);
	~MyApp();

	void setPosition(const sf::Vector2f& pos) override;
	sf::Vector2f getPosition() override { return cardShape.getPosition(); }

	sf::FloatRect getLocalBounds() { return cardShape.getLocalBounds(); }

	enum CLICKBACK
	{
		None,
		DeleteFiles,
		RedownloadFiles,
		OpenFiles
	};

	int onClick(sf::Vector2f clickPos);

	// TODO: switch cardhshape to vertex
	sf::RectangleShape cardShape;
	sf::RectangleShape downloadButton;
	sf::CircleShape	   redownloadButton;
	sf::RectangleShape removeButton;
	sf::RectangleShape launchButton;
	//	sf::RectangleShape controlBar;

	//	bool isRunning();

	bool checkForUpdate(sf::Ftp& ftp);

	bool deleteFilesPrompt();
	void deleteFiles();
	void redownload();
	void download();
	void openItem();

	// TODO: void openItemInStore();

	void updateSizeAndPosition(float xSize, float ySize, float xPos, float yPos);

	void update() override;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Texture		   iconTexture;
	sf::RectangleShape icon;

	sf::Texture downloadButtonTexture;
	sf::Texture redownloadButtonTexture;
	sf::Texture removeButtonTexture;
	sf::Texture launchButtonTexture;

	std::string itemInstallDir;
	sf::Font	font;
	sf::Text	name;
	sf::Text	description;
	sf::Text	version;

	void parseInfo(std::string dir);

	bool downloadIcon();
	bool downloadInfo();
	bool downloadFiles();
};

#endif // !APP_HPP
