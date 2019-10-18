#ifndef APP_EDIT_STATE_HPP
#define APP_EDIT_STATE_HPP

#include "AppState.hpp"
#include "App.hpp"
#include "ToolTip.hpp"

#include "../SFUI-Whorehouse/SettingsParser.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/InputBox.hpp>
#include <SFUI/CheckBox.hpp>

#include <SFML/Graphics.hpp>
#include <vector>

class AppEditState : public AppState
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
	AppEngine* appEngine;

	SFUI::Menu* menu;

	SettingsParser itemInfoParser;

	SFUI::InputBox* appName;
	SFUI::InputBox* appDescription;
	SFUI::InputBox* appVersion;
	SFUI::InputBox* author;
	SFUI::InputBox* github;
	SFUI::InputBox* iconFilePath;
	SFUI::InputBox* dataFilePath;

	SFUI::CheckBox* copylocalfiles;

	AppInfo app;
	App* appPreview;

	bool newApp = false;

	sf::ConvexShape triangle;
	Tooltip tooltip;

	std::string iconTipString = "Icon file path is inavlid.";
	std::string dataTipString = "Data file path is invalid.";

	bool infoUploadFailed;
	bool uploadComplete = false;

	bool iconUploadFailed;
	bool iconPathValid = false;
	bool iconAlreadyHere = false;

	bool dataUploadFailed;
	bool dataPathValid = false;

	// loads information and prepares this app to edit that app
	void prepareToEdit(size_t appid);

	// builds the SFUI menu
	void buildMenu();

	// returns the newly allocated appid
	int registerNewApp();

	bool downloadAppInfo(const std::string& path, const std::string& appid_s, int appid);

	void saveAppData();

	// creates local files for a new app
	// these are what will be uploaded to the server when the app is completed
	void createNewAppFiles(int appid);

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Text &object);
};

#endif // !APP_EDIT_STATE_HPP