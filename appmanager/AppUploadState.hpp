#ifndef APP_UPLOAD_STATE_HPP
#define APP_UPLOAD_STATE_HPP

#include "AppState.hpp"
#include "App.hpp"
#include "ToolTip.hpp"

#include "../SFUI-Whorehouse/SettingsParser.hpp"

#include <SFUI/Layouts/Menu.hpp>
#include <SFUI/InputBox.hpp>
#include <SFML/Graphics.hpp>
#include <vector>

class AppUploadState : public AppState
{
public:
	AppUploadState(AppInfo app, bool ignoreIcon, bool ignoreData, bool copylocalfiles/*, bool newApp*/);

	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

private:
	AppEngine* app;

	AppInfo appToUpload;

	std::vector<std::string> messages;

	bool copylocalfiles = false;
	bool ignoreIcon = false;
	bool ignoreData = false;
//	bool updateAppName = false;

	void uploadApp();

	bool uploadComplete = false;
	bool iconUploadFailed = false;
	bool infoUploadFailed = false;
	bool dataUploadFailed = false;
};

#endif // !APP_UPLOAD_STATE_HPP