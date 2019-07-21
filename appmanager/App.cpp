#include "App.hpp"

#include "Download.hpp"
#include "Globals.hpp"

#include <SFUI/Theme.hpp>
#include <iostream>

App::App(sf::RenderWindow* target_window, float xSize, float ySize, float xPos, float yPos)
{
	sf::Clock itemCreateTimer;

	std::cout << "creating new card" << std::endl;

	targetWindow = target_window;

	cardShape.setSize(sf::Vector2f(xSize, ySize));
	cardShape.setPosition(sf::Vector2f(xPos, yPos)); // probably not the best
	cardShape.setFillColor(sf::Color(100, 100, 100));

	totalHeight = cardShape.getSize().y;

	icon.setSize(sf::Vector2f(cardShape.getSize().y, cardShape.getSize().y)); // a square
	icon.setPosition(cardShape.getPosition());

	name.setFont(SFUI::Theme::getFont());
	description.setFont(SFUI::Theme::getFont());
	version.setFont(SFUI::Theme::getFont());

	name.setCharacterSize(24);
	description.setCharacterSize(16);
	version.setCharacterSize(18);

	name.setPosition(static_cast<int>(icon.getPosition().x + icon.getSize().x + 10), static_cast<int>(cardShape.getPosition().y));
	description.setPosition(static_cast<int>(icon.getPosition().x + icon.getSize().x + 10), static_cast<int>(cardShape.getPosition().y) + 26);
	version.setPosition(static_cast<int>(icon.getPosition().x + icon.getSize().x + 10), static_cast<int>(cardShape.getPosition().y) + 46);

	name.setFillColor(sf::Color(255, 255, 255));
	description.setFillColor(sf::Color(255, 255, 255));
	version.setFillColor(sf::Color(255, 255, 255));

	float fuckedUpXPosition = (cardShape.getPosition().x + cardShape.getLocalBounds().width - 30);

	if (!downloadButtonTexture.loadFromFile("../SFUI-Whorehouse/bin/resources/textures/get_app_1x.png"))
		downloadButton.setFillColor(sf::Color(sf::Color::Green));
	else
		downloadButton.setFillColor(sf::Color(255, 255, 255));
	downloadButtonTexture.setSmooth(true);
	downloadButton.setTexture(&downloadButtonTexture);
	downloadButton.setSize(sf::Vector2f(24, 24));
	downloadButton.setOrigin(sf::Vector2f(downloadButton.getLocalBounds().width / 2, downloadButton.getLocalBounds().height / 2));
	downloadButton.setPosition(sf::Vector2f(fuckedUpXPosition, cardShape.getPosition().y + (cardShape.getSize().y / 2)));

	std::cout << "card is ready (took " << itemCreateTimer.getElapsedTime().asSeconds() << " seconds)" << std::endl;
}

App::~App()
{
	// don't delete targetwindow because it's just a pointer and if we delete it it's actually deleting the app's renderwindow
	// and i spent all day debugging this one problem but it was just the destructor and wowiee fuck me
}

void App::setAppInfo(AppInfo appinfo)
{
	info = appinfo;

	name.setString(info.name);
	description.setString(info.description);
	version.setString(info.version);

	if (!iconTexture.loadFromFile(GBL::DIR::apps + std::to_string(info.appid) + "/icon.png"))
	{
		// TODO: give 'em pink squares
		std::cerr << "failed to load appicon" << std::endl;
	}
	else
		icon.setTexture(&iconTexture);
}

void App::draw()
{
	targetWindow->draw(cardShape);
	targetWindow->draw(icon);

	targetWindow->draw(name);
	targetWindow->draw(description);
	targetWindow->draw(version);

	targetWindow->draw(downloadButton);
}
