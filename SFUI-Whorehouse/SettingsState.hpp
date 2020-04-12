#ifndef SETTINGS_STATE_HPP
#define SETTINGS_STATE_HPP

#include "AppState.hpp"
#include "ColorInputBox.hpp"
#include "SettingsParser.hpp"
#include "VerticalScrollbar.hpp"
#include "Theme.hpp"

#include <SFUI/SFUI.hpp>

#include <SFML/Graphics.hpp>

// TODO: add button to reset settings

class SettingsState : public AppState
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

	// TODO: retain scroll position after menu changes
	VerticalScrollbar scrollbar;
	sf::Vector2f originalMenuPosition;
	sf::Vector2f originalThumbPosition;
	float scrollerTopPosition;
	float scrollerBottomPosition;
	float scrollerMinPosition;
	float scrollerMaxPosition;
	// updates the scrollthumb's size with new content height
	void updateScrollThumbSize();
	// updates the viewScroller's physical limits
	void updateScrollLimits();
	void testScrollBounds();
	// TODO: get rid of this
	void menuMove(const sf::Vector2f& offset) 
	{
		menu->setPosition(sf::Vector2f(menu->getAbsolutePosition().x + offset.x, menu->getAbsolutePosition().y + offset.y));
	}

	SettingsParser configParser;

	void buildDefaultMenu();
	void buildIntersitial();
	void buildThemeEditor(bool editingCurrentTheme);
	void buildIssueReporter();

	// Save all current settings to kunlauncher.conf
	void saveAllSettings();
	// Load settings from kunlauncher.conf
	// TODO: watch for file changes and automatically reload settings
	void loadSettings();

	enum class Menu
	{
		Main,
		ThemeIntersitial,
		ThemeEditor,
		IssueReporter
	} currentMenu;

	SFUI::Menu* menu;

	// TODO: support overwritten files
	struct ThemeEditor
	{
		SFUI::InputBox* name;
		SFUI::InputBox* author;
		SFUI::InputBox* release;

		ColorInputBox* primaryColor;
		ColorInputBox* secondaryColor;
		ColorInputBox* tertiaryColor;

		// TODO: implement this
		ColorInputBox* text;
		ColorInputBox* textSecondary;

		ColorInputBox* scrolltrack;
		ColorInputBox* scrollthumb;
		ColorInputBox* scrollthumb_hover;
		ColorInputBox* scrollthumb_hold;

		ColorInputBox* app_card;
		ColorInputBox* app_card2;
		ColorInputBox* app_card_image;
		ColorInputBox* app_card_icon;
		ColorInputBox* app_card_icon_hover;
		ColorInputBox* app_card_icon_press;
		ColorInputBox* app_card_icon_fail;
		ColorInputBox* app_card_icon_fail_hover;
		ColorInputBox* app_card_icon_fail_press;
		ColorInputBox* app_card_icon_warn;
		ColorInputBox* app_card_icon_warn_hover;
		ColorInputBox* app_card_icon_warn_press;

		SFUI::InputBox* overridenFiles;
		std::string originalOverridenFiles;

		bool changesSaved;

		Theme theme;
	} editor;

	struct SettingsMenu
	{
		SFUI::CheckBox* updateOnStartCheck;
		SFUI::CheckBox* logDownloadsCheck;
		SFUI::CheckBox* newsEnabledCheck;
		SFUI::CheckBox* debugTextureCheck;
		SFUI::CheckBox* allowStatTrackingCheck;
		SFUI::CheckBox* useAnimationsCheck;
		SFUI::InputBox* animationScaleBox;

		SFUI::OptionsBox<std::string>* selectedThemeOptions;

		SFUI::CheckBox* verticalSyncCheck;
		// TODO: make this work
		SFUI::InputBox* widthBox;
		// TODO: make this work
		SFUI::InputBox* heightBox;

		SFUI::CheckBox* apps_updateListOnStartCheck;
		SFUI::CheckBox* apps_autoUpdate;
		SFUI::CheckBox* apps_checkForUpdates;
	} main;

	struct IssueReporter
	{
		SFUI::InputBox* name;
		SFUI::InputBox* description;
		SFUI::InputBox* attachments;

		SFUI::CheckBox* includeSystemInformation;
	} reporter;

	void updateOverridenTextures();
	void applyTheme();
	bool promptExitWithoutSavingChanges();
	bool failedToSaveDialog();

	std::vector<std::string> getDirectories(const std::string& s);

	bool mouseIsOver(const sf::Shape &object);
	bool mouseIsOver(const sf::Shape &object, const sf::View* view);
	bool mouseIsOver(const sf::Text &object);
};

#endif // !SETTINGS_STATE_HPP
