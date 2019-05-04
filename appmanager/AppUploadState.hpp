#ifndef APP_UPLOAD_STATE_HPP
#define APP_UPLOAD_STATE_HPP

#include "../SFUI-Whorehouse/AppState.hpp"
#include "App.hpp"
#include "../SFUI-Whorehouse/SettingsParser.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/InputBox.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

class Tooltip : public sf::Drawable
{
public:
	void setPosition(sf::Vector2f pos);
	void setString(std::string string, float windowSize);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	sf::Text text;
	sf::ConvexShape arrow;
	sf::RectangleShape box;

	const float padding = 2;

	void wrapSfText(sf::Text& target, const float width);
};

class AppUploadState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

private:
	AppEngine* app;

	SFUI::Menu* menu;

	SettingsParser itemInfo;

	SFUI::InputBox* appName;
	SFUI::InputBox* appDescription;
	SFUI::InputBox* appVersion;
	SFUI::InputBox* author;
	SFUI::InputBox* github;
	SFUI::InputBox* iconFilePath;
	SFUI::InputBox* dataFilePath;

	bool creatingApp = false;

	App* appPreview;

	sf::ConvexShape triangle;
	Tooltip *toolTip;

	std::string iconTipString = "Icon file path is inavlid.";
	std::string dataTipString = "Data file path is invalid.";

	bool infoUploadFailed;
	bool uploadComplete = false;

	bool iconUploadFailed;
	bool iconPathValid = false; 

	bool dataUploadFailed;
	bool dataPathValid = false;

	void addNewApp();
	void prepareNewApp();
	void uploadApp();

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Text &object);
};

#endif // !APP_UPLOAD_STATE_HPP