#include "AppEngine.hpp"
#include "SettingsState.hpp"
#include "HomeState.hpp"
#include "MyAppListState.hpp"
#include "AllAppsListState.hpp"

#include "Globals.hpp"
#include "SettingsParser.hpp"
#include "MessageBox.hpp"

#include <SFUI/SFUI.hpp>
#include <SFUI/Theme.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ctime>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

std::string RGBtoStr(sf::Color rgb)
{
	return std::to_string(rgb.r) + "," + std::to_string(rgb.g) + "," + std::to_string(rgb.b);
}

enum CALLBACK
{
	THEME_TEXTURE_OVERRIDE,

	UPDATE_ON_START,
	LOG_DOWNLOADS,
	NEWS_ENABLED,
	SFUI_DEBUG_TEXTURE,
	ALLOW_STAT_TRACKING,
	USE_ANIMATIONS,
	ANIMATION_SCALE,

	SELECTED_THEME,

	VERTICAL_SYNC,
	WIDTH,
	HEIGHT,

	APP_UPDATE_LIST_ON_START,
	APP_AUTO_UPDATE,
	APP_CHECK_FOR_UPDATES,

	TO_THEME_EDITOR,
	REPORT_ISSUE,
	THIRD_PARTY_NOTICES
};

enum CALLBACK_INTERSITIAL
{
	CREATE_NEW_THEME,
	EDIT_THEME,
	EDIT_CURRENT_THEME,
	INT_BACK
};

// TODO: improve the theme editor
enum CALLBACK_THEME_EDITOR
{
	THEME_EDITOR_NAME,
	THEME_EDITOR_AUTHOR,
	THEME_EDITOR_VERSION,
	THEME_EDITOR_OVERRIDE_FILES,
	EDITOR_BACK,
	THEME_NAME,
	SAVE_THEME,
	SAVE_CURRENT_THEME,
};

enum ISSUE_REPORTER_CALLBACK
{
	CANCEL,
	SEND
};

void SettingsState::Init(AppEngine* app_)
{
	std::cout << "SettingsState Init" << std::endl;
	app = app_;

	navbar = new Navbar(app->window);
	navbar->addSection("home");
	navbar->addSection("my apps");
	navbar->addSection("all apps");
	navbar->addSection("settings").setStyle(sf::Text::Style::Bold);

	scrollbar.create(app->window);
	scrollbar.setTrackHeight(app->window->getSize().y - navbar->bar.getLocalBounds().height);
	scrollbar.setPosition(sf::Vector2f(scrollbar.getPosition().x, navbar->bar.getSize().y));

	configParser.loadFromFile(GBL::CONFIG::config);

	buildDefaultMenu();
}

void SettingsState::Cleanup()
{
	std::cout << "Cleaning up SettingsState." << std::endl;

	delete menu;
	delete navbar;

	std::cout << "Cleaned up SettingsState." << std::endl;
}

void SettingsState::Pause()
{
	std::cout << "SettingsState Pause" << std::endl;
}

void SettingsState::Resume()
{
	std::cout << "SettingsState Resume" << std::endl;
}

void SettingsState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		navbar->HandleEvents(event);

		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
			return;
		}
		else if (event.type == sf::Event::EventType::Resized)
		{
			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				app->window->setView(sf::View(visibleArea));
			}
			else
			{
				if (event.size.width <= 525)
					newSize.x = 525;

				if (event.size.height <= 325)
					newSize.y = 325;

				app->window->setSize(newSize);
			}

			scrollbar.setTrackHeight(app->window->getSize().y - navbar->bar.getLocalBounds().height);
			scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, navbar->bar.getLocalBounds().height));
			updateScrollThumbSize();

			menu->setPosition(sf::Vector2f(10, navbar->bar.getSize().y + 10));

			app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));
		}
		else if (event.type == sf::Event::EventType::MouseButtonPressed)
		{
			if (event.key.code == sf::Mouse::Button::Left)
			{
				if (mouseIsOver(navbar->bar))
				{
					for (auto& x : navbar->sections)
						if (x.getString() != "settings" && mouseIsOver(x))
						{
							if (currentMenu == Menu::ThemeEditor)
								if (!editor.changesSaved)
									promptExitWithoutSavingChanges();

							if (x.getString() == "home")
								app->ChangeState(new HomeState);
							else if (x.getString() == "my apps")
								app->ChangeState(new MyAppListState);
							else if (x.getString() == "all apps")
								app->ChangeState(new AllAppsListState);

							std::cout << "state will be switched" << std::endl;
							return;
						}
				}
			}
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (currentMenu != Menu::Main && currentMenu != Menu::IssueReporter && currentMenu != Menu::ThemeIntersitial)
			{
				if (event.key.code == sf::Keyboard::Key::S)
				{
					if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
					{
						if (!editor.theme.saveToFile())
						{
							std::cerr << "failed to save theme (keybind)" << std::endl;

							MessageBox::Options modOptions;
							modOptions.title = "Error";
							modOptions.text = "Failed to save theme.";
							modOptions.settings = { "Ignore", "Retry" };

							MessageBox failedToSave(modOptions);
							failedToSave.runBlocking();

							// TODO: finish failed to save prompt
							if (failedToSave.exitCode == 1) // retry
							{
								if (!editor.theme.saveToFile())
								{
									std::cerr << "failed to save theme (keybind)" << std::endl;

									MessageBox::Options modOptions2;
									modOptions2.title = "Error";
									modOptions2.text = "Failed to save theme.";
									modOptions2.settings = { "Give up." };

									MessageBox failedToSave2(modOptions2);
									failedToSave2.runBlocking();
								}
							}
						}
					}
				}
			}
		}

		// FIXME: this is complete garbage
		// find a way to do this without a loop, preferrably
		for (size_t i = 0; i < 1; i++)
		{
			// FIXME: sometimes this doesn't work correctly
			// after selecting an element and then deselecting it,
			// pressing either of these buttons will select the top or bottom element
			// perhaps hasFocusedWidgetInside() is bugged

			if (!menu->hasFocusedWidgetInside())
				if (event.type == sf::Event::EventType::KeyPressed)
					if (event.key.code == sf::Keyboard::Key::Up || event.key.code == sf::Keyboard::Key::Down)
						break;

			if (currentMenu == Menu::Main)
			{
				int id = menu->onEvent(event);
				switch (id)
				{
				case CALLBACK::UPDATE_ON_START:
					app->settings.updateOnStart = main.updateOnStartCheck->isChecked();
					configParser.set(GBL::CONFIG::updateOnStart, app->settings.updateOnStart);
					break;
				case CALLBACK::LOG_DOWNLOADS:
					app->settings.logDownloads = main.logDownloadsCheck->isChecked();
					configParser.set(GBL::CONFIG::logDownloads, app->settings.logDownloads);
					break;
				case CALLBACK::NEWS_ENABLED:
					app->settings.news.enabled = main.newsEnabledCheck->isChecked();
					configParser.set(GBL::CONFIG::News::newsEnabled, app->settings.news.enabled);
					break;
				case CALLBACK::SFUI_DEBUG_TEXTURE:
					// TODO: don't use solid string here
					SFUI::Theme::loadTexture(GBL::DIR::textures + "interface_square.png", main.debugTextureCheck->isChecked());
					app->settings.SFUIDebug = main.debugTextureCheck->isChecked();
					configParser.set(GBL::CONFIG::SFUIDebug, main.debugTextureCheck->isChecked());
					break;
				case CALLBACK::ALLOW_STAT_TRACKING:
					app->settings.allowStatTracking = main.allowStatTrackingCheck->isChecked();
					configParser.set(GBL::CONFIG::allowStatTracking, app->settings.allowStatTracking);
					break;
				case CALLBACK::USE_ANIMATIONS:
					app->settings.useAnimations = main.useAnimationsCheck->isChecked();
					configParser.set(GBL::CONFIG::useAnimations, app->settings.useAnimations ? "TRUE" : "FALSE");
					break;
				case CALLBACK::ANIMATION_SCALE:
				{
					std::string s = main.animationScaleBox->getText();
					app->settings.animationScale = std::stoi(s);
					configParser.set(GBL::CONFIG::animationScale, app->settings.animationScale);
					break;
				}
				case CALLBACK::SELECTED_THEME:
				{
					applyTheme();
					buildDefaultMenu();
					menu->focusWidget(main.selectedThemeOptions);
					break;
				}
				case CALLBACK::VERTICAL_SYNC:
					std::cout << "vsync" << std::endl;
					app->settings.window.verticalSync = main.verticalSyncCheck->isChecked();
					app->window->setVerticalSyncEnabled(app->settings.window.verticalSync);
					configParser.set(GBL::CONFIG::Window::verticalSync, app->settings.window.verticalSync ? "TRUE" : "FALSE");
					break;
				case CALLBACK::WIDTH:
				{
					std::string s = main.widthBox->getText();
					app->settings.window.width = std::stoi(s);

					if (app->settings.window.width < 525)
					{
						app->settings.window.width = 525;
						main.heightBox->setText("325");
					}

					configParser.set(GBL::CONFIG::Window::width, app->settings.window.width);
					app->window->setSize(sf::Vector2u(app->settings.window.width, app->settings.window.height));
					break;
				}
				case CALLBACK::HEIGHT:
				{
					std::string s = main.heightBox->getText();
					app->settings.window.height = std::stoi(s);

					if (app->settings.window.height < 325)
					{
						app->settings.window.height = 325;
						main.heightBox->setText("325");
					}

					configParser.set(GBL::CONFIG::Window::height, app->settings.window.height);
					app->window->setSize(sf::Vector2u(app->settings.window.width, app->settings.window.height));
					break;
				}
				case CALLBACK::APP_UPDATE_LIST_ON_START:
					app->settings.apps.updateStoreOnStart = main.apps_updateListOnStartCheck->isChecked();
					configParser.set(GBL::CONFIG::Apps::updateStoreOnStart, app->settings.apps.updateStoreOnStart);
					break;
				case CALLBACK::APP_AUTO_UPDATE:
					app->settings.apps.autoUpdate = main.apps_autoUpdate->isChecked();
					configParser.set(GBL::CONFIG::Apps::autoUpdate, app->settings.apps.autoUpdate);
					break;
				case CALLBACK::APP_CHECK_FOR_UPDATES:
					app->settings.apps.checkForUpdates = main.apps_checkForUpdates->isChecked();
					configParser.set(GBL::CONFIG::Apps::checkForUpdates, app->settings.apps.checkForUpdates);
					break;

				case CALLBACK::TO_THEME_EDITOR:
					buildIntersitial();
					break;
				case CALLBACK::REPORT_ISSUE:
					buildIssueReporter();
					break;
				case CALLBACK::THIRD_PARTY_NOTICES:
#ifdef _WIN32
					std::cout << "opening item" << std::endl;
					std::string launch = "start " + GBL::DIR::installDir + "thirdpartynotices.txt";
					system(launch.c_str());
#else
					GBL::MESSAGES::cantOpenNotWindows();
#endif
					break;
				};
			}
			else if (currentMenu == Menu::ThemeIntersitial)
			{
				int id = menu->onEvent(event);
				switch (id)
				{
				case CALLBACK_INTERSITIAL::INT_BACK:
					buildDefaultMenu();
					break;
				case CALLBACK_INTERSITIAL::EDIT_CURRENT_THEME:
					buildThemeEditor(true);
					break;
				case CALLBACK_INTERSITIAL::CREATE_NEW_THEME:
					buildThemeEditor(false);
					break;
				};
			}
			else if (currentMenu == Menu::ThemeEditor)
			{
				// TODO: optimise the shit out of this
				if (event.type == sf::Event::TextEntered)
				{
					navbar->bar.setFillColor(editor.theme.palatte.PRIMARY);
					for (auto& x : navbar->sections)
						x.setFillColor(editor.theme.palatte.TEXT);

					SFUI::Theme::click.textColor = editor.theme.palatte.TEXT_SECONDARY;
					SFUI::Theme::click.textColorHover = editor.theme.palatte.TEXT_SECONDARY;
					SFUI::Theme::click.textColorFocus = editor.theme.palatte.TEXT_SECONDARY;
					SFUI::Theme::input.textColor = editor.theme.palatte.TEXT_SECONDARY;
					SFUI::Theme::input.textColorHover = editor.theme.palatte.TEXT_SECONDARY;
					SFUI::Theme::input.textColorFocus = editor.theme.palatte.TEXT_SECONDARY;

					editor.theme.palatte.PRIMARY = editor.primaryColor->getColor();
					editor.theme.palatte.SECONDARY = editor.secondaryColor->getColor();
					editor.theme.palatte.TERTIARY = editor.tertiaryColor->getColor();

					editor.theme.palatte.TEXT = editor.text->getColor();
					editor.theme.palatte.TEXT_SECONDARY = editor.textSecondary->getColor();

					editor.theme.palatte.SCROLLBAR = editor.scrolltrack->getColor();
					scrollbar.scrollTrack.setFillColor(editor.theme.palatte.SCROLLBAR);
					editor.theme.palatte.SCROLLTHUMB = editor.scrollthumb->getColor();
					scrollbar.scrollThumb.setFillColor(editor.theme.palatte.SCROLLTHUMB);
					editor.theme.palatte.SCROLLTHUMB_HOVER = editor.scrollthumb_hover->getColor();
					editor.theme.palatte.SCROLLTHUMB_HOLD = editor.scrollthumb_hold->getColor();

					editor.theme.palatte.APP_CARD = editor.app_card->getColor();
					editor.theme.palatte.APP_CARD2 = editor.app_card2->getColor();
					editor.theme.palatte.APP_IMAGE = editor.app_card_image->getColor();
					editor.theme.palatte.APP_ICON = editor.app_card_icon->getColor();
					editor.theme.palatte.APP_ICON_HOVER = editor.app_card_icon_hover->getColor();
					editor.theme.palatte.APP_ICON_PRESS = editor.app_card_icon_press->getColor();
					editor.theme.palatte.APP_ICON_FAIL = editor.app_card_icon_fail->getColor();
					editor.theme.palatte.APP_ICON_FAIL_HOVER = editor.app_card_icon_fail_hover->getColor();
					editor.theme.palatte.APP_ICON_FAIL_PRESS = editor.app_card_icon_fail_press->getColor();
					editor.theme.palatte.APP_ICON_WARN = editor.app_card_icon_warn->getColor();
					editor.theme.palatte.APP_ICON_WARN_HOVER = editor.app_card_icon_warn_hover->getColor();
					editor.theme.palatte.APP_ICON_WARN_PRESS = editor.app_card_icon_warn_press->getColor();
				}

				int id = menu->onEvent(event);

				if (id != -1)
					editor.changesSaved = false;

				switch (id)
				{
				case THEME_EDITOR_OVERRIDE_FILES:
					std::cout << "updating overriden textures" << std::endl;
					updateOverridenTextures();
					break;
				case SAVE_THEME:
					editor.theme.saveToFile();
					break;
				case SAVE_CURRENT_THEME:
					updateOverridenTextures();
					GBL::theme = editor.theme;
					GBL::theme.saveToFile();
					break;
				case CALLBACK_THEME_EDITOR::EDITOR_BACK:
					if (!editor.changesSaved)
						if (promptExitWithoutSavingChanges())
							editor.theme.saveToFile();

					buildIntersitial();
					break;
				};
			}
			else if (currentMenu == Menu::IssueReporter)
			{
				int id = menu->onEvent(event);
				switch (id)
				{
				case ISSUE_REPORTER_CALLBACK::CANCEL:
					buildDefaultMenu();
					break;
				case ISSUE_REPORTER_CALLBACK::SEND:
				{
					// TODO: someway to notify me when an issue is submitted

					std::string date = app->currentDateTime();
					std::string file = GBL::DIR::installDir + "iss" + date + ".txt";

					std::ofstream issue(file, std::ios::out, std::ios::trunc);
					if (issue.is_open())
					{
						issue.close();

						SettingsParser sp;
						if (sp.loadFromFile(file))
						{
							sp.set("name", std::string(reporter.name->getText()));
							sp.set("description", std::string(reporter.description->getText()));
							sp.set("date", date);
							sp.set("version", GBL::version.asString());

							// system information

							// TODO: upload class
							sf::Ftp ftp;

							sf::Ftp::Response response = ftp.connect("files.000webhost.com", 21, sf::milliseconds(10000));
							if (!response.isOk())
							{
								std::cerr << "failed to connect to ftp (" << response.getMessage() << " (" << response.getStatus() << "))" << std::endl;
								break;
							}

							response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
							if (!response.isOk())
							{
								std::cerr << "failed to login to ftp" << std::endl;
								break;
							}

							response = ftp.changeDirectory("public_html");
							if (!response.isOk())
							{
								std::cerr << "failed to set ftp directory" << std::endl;
								break;
							}

							response = ftp.upload(file, GBL::WEB::ISSUES);
							if (!response.isOk())
							{
								std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;
								break;
							}
							else
							{
								std::cout << "uploaded issue" << std::endl;

								try
								{
									fs::remove(file);
								}
								catch (const std::exception& e)
								{
									std::cerr << "failed to delete local issue" << std::endl;
									std::cerr << e.what() << std::endl;
								}
							}
						}
						else
						{
							std::cerr << "failed to open local issue for writing" << std::endl;
							break;
						}
					}
					else
					{
						std::cerr << "failed to create local issue" << std::endl;
						break;
					}

					break;
				}
				}
			}
		}

		// TODO: scrollbar control class
		// TODO: scroll up if widget is out of bounds
		if (scrollbar.isEnabled && !app->multithreaded_process_running)
//		if (menu->focus == NULL && scrollbar.isEnabled)
		{
			if (event.type == sf::Event::EventType::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Button::Left)
				{
					if (mouseIsOver(scrollbar.scrollThumb))
					{
						scrollbar.dragOffset = scrollbar.scrollThumb.getPosition() - sf::Vector2f(sf::Mouse::getPosition(*app->window));
						scrollbar.draggingThumb = true;
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOLD);

						originalMenuPosition = menu->getAbsolutePosition();
						originalThumbPosition = scrollbar.scrollThumb.getPosition();
					}
				}
			}
			else if (event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (scrollbar.draggingThumb)
				{
					scrollbar.draggingThumb = false;

					if (mouseIsOver(scrollbar.scrollThumb))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
				}
			}
			else if (event.type == sf::Event::EventType::MouseWheelMoved)
			{
				// TODO: PageUp / PageDown
				// if PageUp
				//	viewScroller.move(0, scrollTrack.viewHeight);

				if (event.mouseWheel.delta < 0) // down, or move apps up
				{
					scrollbar.jumpDown();

					if (scrollerBottomPosition > scrollerMaxPosition)
						menuMove(sf::Vector2f(0, -scrollbar.scrollJump));
				}
				else if (event.mouseWheel.delta > 0) // scroll up, or move apps down
				{
					scrollbar.jumpUp();

					if (scrollerTopPosition < scrollerMinPosition)
						menuMove(sf::Vector2f(0, scrollbar.scrollJump));
				}
				
				testScrollBounds();
			}
			else if (event.type == sf::Event::EventType::KeyPressed && !menu->hasFocusedWidgetInside())
			{
				if (event.key.code == sf::Keyboard::Key::Down)
				{
					scrollbar.stepDown();

					if (scrollerBottomPosition > scrollerMaxPosition)
						menuMove(sf::Vector2f(0, -scrollbar.scrollStep)); // static cast to avoid pixel-imperfect placement of text
				}
				else if (event.key.code == sf::Keyboard::Key::Up)
				{
					scrollbar.stepUp();

					if (scrollerTopPosition < scrollerMinPosition)
						menuMove(sf::Vector2f(0, scrollbar.scrollStep)); // static cast to avoid pixel-imperfect placement of text
				}
				else if (event.key.code == sf::Keyboard::Key::Home)
				{
					scrollbar.moveToTop();
					menu->setPosition(sf::Vector2f(10, navbar->bar.getSize().y + 10));
				}
				else if (event.key.code == sf::Keyboard::Key::End)
				{
					scrollbar.moveToBottom();
					menu->setPosition(sf::Vector2f(10, -menu->getSize().y + app->window->getSize().y - 10));
				}
				
				testScrollBounds();
			}
			else if (event.type == sf::Event::EventType::MouseMoved)
			{
				if (scrollbar.draggingThumb)
				{
					scrollbar.scrollThumb.setPosition(sf::Vector2f(scrollbar.scrollThumb.getPosition().x, sf::Mouse::getPosition(*app->window).y + scrollbar.dragOffset.y));
					if (!scrollbar.boundsCheck()) // scroll from the last legitimate location
						scrollbar.dragOffset = scrollbar.scrollThumb.getPosition() - sf::Vector2f(sf::Mouse::getPosition(*app->window));

					menu->setPosition(sf::Vector2f( menu->getAbsolutePosition().x, originalMenuPosition.y - ((scrollbar.scrollThumb.getPosition().y - originalThumbPosition.y) * scrollbar.scrollStep) ));

//					FIXME: error zone for scrollbar (settingsstate)
//					if (sf::Mouse::getPosition(*app->window).x > scrollbar.scrollTrack.getPosition().x + 150 || 
//						sf::Mouse::getPosition(*app->window).x < scrollbar.scrollTrack.getPosition().x - 150) // error zone
//						scrollbar.scrollThumb.setPosition(scrollbar.originalPosition);
				}
				else
				{
					// TODO: find a way to not change scrollbar color every update

					if (mouseIsOver(scrollbar.scrollThumb))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
				}
			}
		}
	}
}

void SettingsState::Update()
{
}

void SettingsState::Draw()
{
	app->window->clear(GBL::theme.palatte.TERTIARY);

	app->window->draw(*menu);

	navbar->Draw();
	app->window->draw(scrollbar);

	app->window->display();
}

void SettingsState::buildDefaultMenu()
{
	std::cout << "building default menu" << std::endl;

	delete menu;
	menu = new SFUI::Menu(*app->window);
	menu->setPosition(sf::Vector2f(10, navbar->bar.getSize().y + 10));
	// TODO: rename this to currentMenuState
	currentMenu = Menu::Main;

	SFUI::FormLayout* form = menu->addFormLayout();

	form->addLabel("Application Settings");

	main.updateOnStartCheck = new SFUI::CheckBox(app->settings.updateOnStart);
	form->addRow("Update on Start", main.updateOnStartCheck, CALLBACK::UPDATE_ON_START);

	main.logDownloadsCheck = new SFUI::CheckBox(app->settings.logDownloads);
	form->addRow("Log Downloads", main.logDownloadsCheck, CALLBACK::LOG_DOWNLOADS);

	main.newsEnabledCheck = new SFUI::CheckBox(app->settings.news.enabled);
	form->addRow("Enable News", main.newsEnabledCheck, CALLBACK::NEWS_ENABLED);

	main.debugTextureCheck = new SFUI::CheckBox(app->settings.SFUIDebug);
	form->addRow("Debug Texture", main.debugTextureCheck, CALLBACK::SFUI_DEBUG_TEXTURE);

	main.allowStatTrackingCheck = new SFUI::CheckBox(app->settings.allowStatTracking);
	form->addRow("Allow Stat Tracking", main.allowStatTrackingCheck, CALLBACK::ALLOW_STAT_TRACKING);

//	main.useAnimationsCheck = new SFUI::CheckBox(app->settings.useAnimations);
//	form->addRow("Use Animations", main.useAnimationsCheck, CALLBACK::USE_ANIMATIONS);

//	main.animationScaleBox = new SFUI::InputBox(50);
//	main.animationScaleBox->setText(std::to_string(app->settings.animationScale));
//	form->addRow("Animation Scale", main.animationScaleBox, CALLBACK::ANIMATION_SCALE);

	main.selectedThemeOptions = new SFUI::OptionsBox<std::string>;
	// each theme has it's own directory in the themes folder
	std::vector<std::string> themes = get_directories(GBL::DIR::themes);
	for (size_t i = 0; i < themes.size(); i++)
	{
		if (themes[i] == app->settings.selectedTheme)
			continue; // don't add the current theme until later

		std::string conf(GBL::DIR::themes + themes[i] + "/" + themes[i] + ".sfuitheme");
		if (fs::exists(conf))
		{
			// get the name
			SettingsParser sp;
			if (!sp.loadFromFile(conf))
			{
				std::cerr << "failed to find theme " << themes[i] << ", skipping" << std::endl;
				continue;
			}

			std::string name;
			sp.get("name", name);

			// add it
			main.selectedThemeOptions->addItem(name, themes[i]);
		}
	}

	main.selectedThemeOptions->addItem(GBL::theme.name, app->settings.selectedTheme); // add the current theme last
	form->addRow("Theme: ", main.selectedThemeOptions, CALLBACK::SELECTED_THEME);


	form->addLabel("Window");

	main.verticalSyncCheck = new SFUI::CheckBox(app->settings.window.verticalSync);
	form->addRow("Vertical Sync", main.verticalSyncCheck, CALLBACK::VERTICAL_SYNC);

	main.widthBox = new SFUI::InputBox(50);
	main.widthBox->setText(std::to_string(app->settings.window.width));
	form->addRow("Width", main.widthBox, CALLBACK::WIDTH);

	main.heightBox = new SFUI::InputBox(50);
	main.heightBox->setText(std::to_string(app->settings.window.height));
	form->addRow("Height", main.heightBox, CALLBACK::HEIGHT);

	form->addLabel("Apps");

	main.apps_updateListOnStartCheck = new SFUI::CheckBox(app->settings.apps.updateStoreOnStart);
	form->addRow("Update Store on Start", main.apps_updateListOnStartCheck, CALLBACK::APP_UPDATE_LIST_ON_START);

	main.apps_autoUpdate = new SFUI::CheckBox(app->settings.apps.autoUpdate);
	form->addRow("Auto Update Apps", main.apps_autoUpdate, CALLBACK::APP_AUTO_UPDATE);

	main.apps_checkForUpdates = new SFUI::CheckBox(app->settings.apps.checkForUpdates);
	form->addRow("Check For Updates", main.apps_checkForUpdates, CALLBACK::APP_CHECK_FOR_UPDATES);

	SFUI::HorizontalBoxLayout* hbox = form->addHorizontalBoxLayout();
	hbox->addButton("Edit Theme...", CALLBACK::TO_THEME_EDITOR);
	hbox->addButton("Report Issue...", CALLBACK::REPORT_ISSUE);
	hbox->addButton("Third Party Notices...", CALLBACK::THIRD_PARTY_NOTICES);

	form->addLabel("KunLauncher\nVersion: " + GBL::version.asString() + "\nBuilt " __DATE__ " at " __TIME__);
	
	updateScrollThumbSize();
}

void SettingsState::buildIntersitial()
{
	delete menu;
	menu = new SFUI::Menu(*app->window);
	menu->setPosition(sf::Vector2f(10, 50));
	currentMenu = Menu::ThemeIntersitial;

	SFUI::HorizontalBoxLayout* hbox = menu->addHorizontalBoxLayout();
	hbox->addButton("Create New Theme", CALLBACK_INTERSITIAL::CREATE_NEW_THEME);
	hbox->addButton("Edit Current Theme", CALLBACK_INTERSITIAL::EDIT_CURRENT_THEME);
	// TODO: pick theme to edit
//	hbox->addButton("Edit Theme...", CALLBACK_INTERSITIAL::EDIT_THEME);
	menu->addButton("Back", CALLBACK_INTERSITIAL::INT_BACK);

	updateScrollThumbSize();
}

void SettingsState::buildThemeEditor(bool editingCurrentTheme)
{
	editor.changesSaved = true;

	delete menu;
	menu = new SFUI::Menu(*app->window);
	menu->setPosition(sf::Vector2f(10, navbar->bar.getSize().y + 10));
	currentMenu = Menu::ThemeEditor;

	SFUI::FormLayout* mainForm = menu->addFormLayout();

	editor.overridenFiles = new SFUI::InputBox;

	if (editingCurrentTheme)
	{
		editor.theme = Theme();
		editor.theme.loadFromFile(app->settings.selectedTheme);

		mainForm->addLabel("Name: " + editor.theme.name);
		mainForm->addLabel("Author: " + editor.theme.author);
		mainForm->addRow("Theme Version: ", editor.version = new SFUI::InputBox, THEME_EDITOR_VERSION);
		editor.version->setText(std::to_string(editor.theme.version));
		mainForm->addLabel("LauncherVersion: " + editor.theme.themeLauncherVersion);  \
		editor.overridenFiles->setText(GBL::theme.overridenFilesAsString());
	}
	else // new theme
	{
		editor.theme = Theme();
		editor.theme.name = "new theme";
		editor.theme.version = 1.0f;
		editor.theme.themeLauncherVersion = GBL::version.asString();

		mainForm->addRow("Name: ", editor.name = new SFUI::InputBox, THEME_NAME);
		mainForm->addRow("Author: ", editor.author = new SFUI::InputBox, THEME_EDITOR_AUTHOR);
		mainForm->addRow("Theme Version: ", editor.version = new SFUI::InputBox, THEME_EDITOR_VERSION);
		mainForm->addLabel("LauncherVersion: " + editor.theme.themeLauncherVersion);
	}

	menu->addHorizontalBoxLayout();
	menu->addHorizontalBoxLayout();

	editor.originalOverridenFiles = GBL::theme.overridenFilesAsString();
//	mainForm->addRow(editor.originalOverridenFiles, editor.overridenFiles, THEME_EDITOR_OVERRIDE_FILES);
	mainForm->addRow("Overriden Files: ", editor.overridenFiles, THEME_EDITOR_OVERRIDE_FILES);

	// TODO: unsaved theme
	// default name is unsaved theme, check for other unsaved themes, and increment number.
	// if we try to save a theme that already exists, prompt the user to override.

	menu->addHorizontalBoxLayout();
	SFUI::HorizontalBoxLayout* hbox = menu->addHorizontalBoxLayout();

	SFUI::VerticalBoxLayout* appColors = hbox->addVerticalBoxLayout();
	SFUI::FormLayout*		 appColorsForm = appColors->addFormLayout();
	appColorsForm->addLabel("Application");

	appColorsForm->addRow("Primary:", editor.primaryColor = new ColorInputBox(75));
	appColorsForm->addRow("Secondary:", editor.secondaryColor = new ColorInputBox(75));
	appColorsForm->addRow("Tertiary:", editor.tertiaryColor = new ColorInputBox(75));
	appColorsForm->addHorizontalBoxLayout();
	appColorsForm->addRow("Text:", editor.text = new ColorInputBox(75));
	appColorsForm->addRow("Text Secondary:", editor.textSecondary = new ColorInputBox(75));

	appColorsForm->addHorizontalBoxLayout();
	appColorsForm->addLabel("Scrollbar");
	appColorsForm->addRow("Bar", editor.scrolltrack = new ColorInputBox(75));
	appColorsForm->addRow("Thumb", editor.scrollthumb = new ColorInputBox(75));
	appColorsForm->addRow("Thumb Hover", editor.scrollthumb_hover = new ColorInputBox(75));
	appColorsForm->addRow("Thumb Hold", editor.scrollthumb_hold = new ColorInputBox(75));

	// Spacing between sections
	hbox->addVerticalBoxLayout();
	hbox->addVerticalBoxLayout();

	// Card Colors
	SFUI::VerticalBoxLayout* cardColors = hbox->addVerticalBoxLayout();
	SFUI::FormLayout*		 cardColorsForm = cardColors->addFormLayout();
	cardColorsForm->addLabel("App Card Colors");
	cardColorsForm->addRow("Background:", editor.app_card = new ColorInputBox(75));
	cardColorsForm->addRow("Background2:", editor.app_card2= new ColorInputBox(75));
	cardColorsForm->addRow("Image:", editor.app_card_image = new ColorInputBox(75));
	cardColorsForm->addRow("Icon:", editor.app_card_icon = new ColorInputBox(75));
	cardColorsForm->addRow("Icon Hover:", editor.app_card_icon_hover = new ColorInputBox(75));
	cardColorsForm->addRow("Icon Press:", editor.app_card_icon_press = new ColorInputBox(75));
	cardColorsForm->addRow("Fail Icon:", editor.app_card_icon_fail = new ColorInputBox(75));
	cardColorsForm->addRow("Fail Icon Hover:", editor.app_card_icon_fail_hover = new ColorInputBox(75));
	cardColorsForm->addRow("Fail Icon Press:", editor.app_card_icon_fail_press = new ColorInputBox(75));
	cardColorsForm->addRow("Warn Icon:", editor.app_card_icon_warn = new ColorInputBox(75));
	cardColorsForm->addRow("Warn Icon Hover:", editor.app_card_icon_warn_hover = new ColorInputBox(75));
	cardColorsForm->addRow("Warn Icon Press:", editor.app_card_icon_warn_press = new ColorInputBox(75));

	menu->addHorizontalBoxLayout();
	menu->addHorizontalBoxLayout();
	menu->addHorizontalBoxLayout();

	if (editingCurrentTheme)
		menu->addButton("Save \"" + editor.theme.name + "\"", CALLBACK_THEME_EDITOR::SAVE_CURRENT_THEME);
	else
		menu->addButton("Save Theme", CALLBACK_THEME_EDITOR::SAVE_THEME);

	menu->addButton("Back", CALLBACK_THEME_EDITOR::EDITOR_BACK);

	if (editingCurrentTheme)
	{
		editor.primaryColor->setColor(GBL::theme.palatte.PRIMARY);
		editor.secondaryColor->setColor(GBL::theme.palatte.SECONDARY);
		editor.tertiaryColor->setColor(GBL::theme.palatte.TERTIARY);

		editor.text->setColor(GBL::theme.palatte.TEXT);
		editor.textSecondary->setColor(GBL::theme.palatte.TEXT_SECONDARY);

		editor.scrolltrack->setColor(GBL::theme.palatte.SCROLLBAR);
		editor.scrollthumb->setColor(GBL::theme.palatte.SCROLLTHUMB);
		editor.scrollthumb_hover->setColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
		editor.scrollthumb_hold->setColor(GBL::theme.palatte.SCROLLTHUMB_HOLD);

		editor.app_card->setColor(GBL::theme.palatte.APP_CARD);
		editor.app_card2->setColor(GBL::theme.palatte.APP_CARD2);
		editor.app_card_image->setColor(GBL::theme.palatte.APP_IMAGE);
		editor.app_card_icon->setColor(GBL::theme.palatte.APP_ICON);
		editor.app_card_icon_hover->setColor(GBL::theme.palatte.APP_ICON_HOVER);
		editor.app_card_icon_press->setColor(GBL::theme.palatte.APP_ICON_PRESS);
		editor.app_card_icon_fail->setColor(GBL::theme.palatte.APP_ICON_FAIL);
		editor.app_card_icon_fail_hover->setColor(GBL::theme.palatte.APP_ICON_FAIL_HOVER);
		editor.app_card_icon_fail_press->setColor(GBL::theme.palatte.APP_ICON_FAIL_PRESS);
		editor.app_card_icon_warn->setColor(GBL::theme.palatte.APP_ICON_WARN);
		editor.app_card_icon_warn_hover->setColor(GBL::theme.palatte.APP_ICON_WARN_HOVER);
		editor.app_card_icon_warn_press->setColor(GBL::theme.palatte.APP_ICON_WARN_PRESS);
	}
	// TODO: else save new theme
	// offer an input box for the name of the theme

	updateScrollThumbSize();
}

void SettingsState::buildIssueReporter()
{
	/* Things we want to collect
		Issue Title --
		Issue Description --
		Theme of user
		OS of user
		System Architecture of user
		System GPU of user
		Time of report --
		LauncherVersion --
		Number of apps downloaded
		Settings of Launcher
	*/

	delete menu;
	menu = new SFUI::Menu(*app->window);
	menu->setPosition(sf::Vector2f(10, navbar->bar.getSize().y + 10));
	currentMenu = Menu::IssueReporter;
	menu->addLabel("Report an Issue");
	menu->addHorizontalBoxLayout();
	
	SFUI::FormLayout* mainForm = menu->addFormLayout();
	mainForm->addRow("Name: ", reporter.name = new SFUI::InputBox);
	mainForm->addRow("Description: ", reporter.description = new SFUI::InputBox);
	mainForm->addLabel("Version: " + GBL::version.asString());
//	mainForm->addRow("Include System Information? ", reporter.includeSystemInformation = new SFUI::CheckBox); // TODO: clean way to get sys info

	/*
	SFUI::HorizontalBoxLayout* hbox1 = menu->addHorizontalBoxLayout();
	hbox1->addButton("Add Attachment...");
	hbox1->addLabel("Attachments: ");
	*/
	menu->addHorizontalBoxLayout();

	SFUI::HorizontalBoxLayout* hbox2 = menu->addHorizontalBoxLayout();
	hbox2->addButton("Cancel", ISSUE_REPORTER_CALLBACK::CANCEL);
	hbox2->addButton("Send Report", ISSUE_REPORTER_CALLBACK::SEND);

	updateScrollThumbSize();
}

void SettingsState::saveAllSettings()
{
	if (configParser.loadFromFile(GBL::CONFIG::config))
	{
		configParser.set(GBL::CONFIG::updateOnStart, app->settings.updateOnStart);
		configParser.set(GBL::CONFIG::logDownloads, app->settings.logDownloads);
		configParser.set(GBL::CONFIG::SFUIDebug, app->settings.SFUIDebug);
		configParser.set(GBL::CONFIG::allowStatTracking, app->settings.allowStatTracking);
		configParser.set(GBL::CONFIG::useAnimations, app->settings.useAnimations);
		configParser.set(GBL::CONFIG::animationScale, app->settings.animationScale);

		configParser.set(GBL::CONFIG::News::newsEnabled, app->settings.news.enabled);

		configParser.set(GBL::CONFIG::selectedTheme, app->settings.selectedTheme);

		configParser.set(GBL::CONFIG::Window::verticalSync, app->settings.window.verticalSync);
		configParser.set(GBL::CONFIG::Window::width, app->settings.window.width);
		configParser.set(GBL::CONFIG::Window::height, app->settings.window.height);

		configParser.set(GBL::CONFIG::Apps::updateStoreOnStart, app->settings.apps.updateStoreOnStart);
		configParser.set(GBL::CONFIG::Apps::autoUpdate, app->settings.apps.autoUpdate);
		configParser.set(GBL::CONFIG::Apps::checkForUpdates, app->settings.apps.checkForUpdates);
	}
	else
	{
		std::cerr << "failed to load configuration for saving." << std::endl;
		return;
	}
}

void SettingsState::loadSettings()
{
	if (configParser.loadFromFile(GBL::CONFIG::config))
	{
		configParser.get(GBL::CONFIG::updateOnStart, app->settings.updateOnStart);
		configParser.get(GBL::CONFIG::logDownloads, app->settings.logDownloads);
		configParser.get(GBL::CONFIG::SFUIDebug, app->settings.SFUIDebug);
		configParser.get(GBL::CONFIG::allowStatTracking, app->settings.allowStatTracking);
		configParser.get(GBL::CONFIG::useAnimations, app->settings.useAnimations);
		configParser.get(GBL::CONFIG::animationScale, app->settings.animationScale);

		configParser.get(GBL::CONFIG::News::newsEnabled, app->settings.news.enabled);

		configParser.get(GBL::CONFIG::selectedTheme, app->settings.selectedTheme);

		configParser.get(GBL::CONFIG::Window::verticalSync, app->settings.window.verticalSync);
		configParser.get(GBL::CONFIG::Window::width, app->settings.window.width);
		configParser.get(GBL::CONFIG::Window::height, app->settings.window.height);

		configParser.get(GBL::CONFIG::Apps::updateStoreOnStart, app->settings.apps.updateStoreOnStart);
		configParser.get(GBL::CONFIG::Apps::autoUpdate, app->settings.apps.autoUpdate);
		configParser.get(GBL::CONFIG::Apps::checkForUpdates, app->settings.apps.checkForUpdates);
	}
	else
	{
		std::cerr << "failed to load configuration for saving." << std::endl;
		return;
	}
}

void SettingsState::updateScrollThumbSize()
{
	const float contentHeight = menu->getSize().y + 20; // padding * 2 = 20

	scrollbar.update(contentHeight, menu->getSize().y - navbar->bar.getSize().y);

	// TODO: update scroll limits separately
	updateScrollLimits();
}

void SettingsState::updateScrollLimits()
{
	scrollerTopPosition = menu->getAbsolutePosition().y;
	scrollerBottomPosition = menu->getAbsolutePosition().y + menu->getSize().y + 10;
	scrollerMinPosition = navbar->bar.getSize().y + 10; // navbar & padding
	scrollerMaxPosition = app->window->getSize().y;
}

void SettingsState::testScrollBounds()
{
	updateScrollLimits();

	if (scrollerBottomPosition < scrollerMaxPosition)
	{
		menu->setPosition(sf::Vector2f(10, -menu->getSize().y + app->window->getSize().y - 10));
		updateScrollLimits();
	}
	else if (scrollerTopPosition > scrollerMinPosition)
	{
		menu->setPosition(sf::Vector2f(10, navbar->bar.getSize().y + 10));
		updateScrollLimits();
	}
}

void SettingsState::updateOverridenTextures()
{
	// get list of current overriden files from text box
	// go through list of textures, if it's there override it.
	// if it's not there and is overriden, remove it.

	editor.theme.updateOverridenFiles(editor.overridenFiles->getText());

	// TODO: text box for this
	SFUI::Theme::loadTexture(editor.theme.getTexture("interface_square.png"));
	app->SetMultiThreadedIndicatorIcon(editor.theme.getTexture("settings_2x.png"));
}

void SettingsState::applyTheme()
{
	if (GBL::theme.loadFromFile(main.selectedThemeOptions->getSelectedValue()))
	{
		app->settings.selectedTheme = main.selectedThemeOptions->getSelectedValue();

		SettingsParser sp;
		sp.loadFromFile(GBL::CONFIG::config);
		sp.set(GBL::CONFIG::selectedTheme, app->settings.selectedTheme);

		//if (GBL::theme.isResourceOverriden("interface_square.png"))
			//SFUI::Theme::loadTexture(GBL::theme.getTexture("interface_square.png"));

		navbar->bar.setFillColor(GBL::theme.palatte.PRIMARY);
		for (auto& x : navbar->sections)
			x.setFillColor(GBL::theme.palatte.TEXT);

		// set the SFUI specific themes
		SFUI::Theme::click.textColor = GBL::theme.palatte.TEXT_SECONDARY;
		SFUI::Theme::click.textColorHover = GBL::theme.palatte.TEXT_SECONDARY;
		SFUI::Theme::click.textColorFocus = GBL::theme.palatte.TEXT_SECONDARY;
		SFUI::Theme::input.textColor = GBL::theme.palatte.TEXT_SECONDARY;
		SFUI::Theme::input.textColorHover = GBL::theme.palatte.TEXT_SECONDARY;
		SFUI::Theme::input.textColorFocus = GBL::theme.palatte.TEXT_SECONDARY;

		scrollbar.scrollTrack.setFillColor(GBL::theme.palatte.SCROLLBAR);
		scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
	}
	else
	{
		// TODO: warn user failed to load theme
	}
}

bool SettingsState::promptExitWithoutSavingChanges()
{
	MessageBox::Options modOptions;
	modOptions.title = "Unsaved Changes";
	modOptions.text = "Your changes have not been saved.\nIf you leave now, any changes will be discarded.";
	modOptions.settings = { "Save", "Discard" };

	MessageBox unsavedChanges(modOptions);
	unsavedChanges.runBlocking();

	if (unsavedChanges.exitCode == 0)
		return true;
	else // literally anything except save
		return false;
}

std::vector<std::string> SettingsState::get_directories(const std::string& s)
{
	std::vector<std::string> r;
	for (auto& p : fs::directory_iterator(s))
		if (p.status().type() == fs::file_type::directory)
			r.push_back(p.path().string().substr(GBL::DIR::themes.size(), p.path().string().size()));
	return r;
}

bool SettingsState::mouseIsOver(const sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

bool SettingsState::mouseIsOver(const sf::Shape &object, const sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool SettingsState::mouseIsOver(const sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}
