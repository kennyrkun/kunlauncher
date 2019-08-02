#ifndef APP_HPP
#define APP_HPP

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

struct ItemInfo
{
	std::string name;
	std::string description;
	// TODO: put end marker character at the end so we know how far the string should go
	// TODO: replace this with a version of GBL::Version
	float version;
	std::string author;
	std::string github;
	int release;
	int appid;

	bool downloaded;
	bool downloading;
	bool missingInfo;
	bool updateAvailable = false;
	struct Status
	{
		bool checkingForUpdate = false;
		bool updateAvailable = false;

		bool downloaded = false;
		bool downloading = false;

		bool missinginfo = false;
		bool missingicon = false;
		bool missingdata = false;

		bool redownloadRequired = false;

		bool isRunning = false;
	} status;

	// TODO: appAPIVersion
};

class App : public sf::Drawable
{
public:
	// returns an int event code on click
	virtual int onClick(sf::Vector2f clickPos) = 0;

	ItemInfo info;

	std::string itemCacheDir;
	std::string itemInstallDir;

	virtual void setPosition(const sf::Vector2f& pos) = 0;
	virtual sf::Vector2f getPosition() = 0;

	sf::RectangleShape cardShape;
	virtual sf::FloatRect getLocalBounds() = 0;

	bool checkForUpdate(sf::Ftp& ftp);

	virtual void updateSizeAndPosition(float xSize, float ySize, float xPos, float yPos) = 0;

	virtual void update() = 0;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const = 0;

protected:
	bool mouseIsOver(const sf::Shape& shape, const sf::Vector2f& position)
	{
		if (shape.getGlobalBounds().contains(position))
			return true;

		return false;
	}

	std::ifstream& GotoLine(std::ifstream& file, unsigned int line);

	virtual void parseInfo(std::string dir) = 0;

	virtual int downloadIcon() = 0;
	virtual int downloadInfo() = 0;
	virtual int downloadFiles() = 0;
};

#endif // !APP_HPP
