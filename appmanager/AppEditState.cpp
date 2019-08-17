#include "AppEngine.hpp"
#include "AppEditState.hpp"
#include "AppUploadState.hpp"
#include "HomeState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"

#include <SFUI/SFUI.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <experimental/filesystem>

// TODO: investigate crash related to : being used in SFUI::InputBox

namespace fs = std::experimental::filesystem;

enum MenuCallbacks
{
	NameTextEntered,
	DescriptionTextEntered,
	VersionTextEntered,
	AuthorTextEntered,
	GitHubTextEntered,
	IconFilePathTextEntered,
	IconFileBrowse,
	DataFilePathTextEntered,
	DataFileBrowse,
	CommitChanges,
	Back,
};

void AppEditState::Init(AppEngine* app_)
{
	std::cout << "AppEditState Init" << std::endl;
	appEngine = app_;

	appEngine->drawInformationPanel("Preparing app editor...");

	if (appEngine->appToEdit == -1)
	{
		std::cout << "editing a new app" << std::endl;

		newApp = true;
		
		appEngine->appToEdit = registerNewApp();

		if (appEngine->appToEdit != -1)
			createNewAppFiles(appEngine->appToEdit);
		else
		{
			std::cerr << "failed to register new appid" << std::endl;
			abort();
		}
	}
	else
	{
		std::cout << "editing existing app" << std::endl;

		if (!fs::exists(GBL::DIR::apps + std::to_string(appEngine->appToEdit)))
		{
			std::cerr << "app info for this app does not exist, we will attempt to download it" << std::endl;

			Download getInfo;
			getInfo.setInput(GBL::WEB::APPS + std::to_string(appEngine->appToEdit) + "/info.dat");
			getInfo.setOutputDir(GBL::DIR::apps + std::to_string(appEngine->appToEdit) + "//");
			getInfo.setOutputFilename("info.dat");

			int status = getInfo.download();

			if (status == Download::Status::Ok)
				getInfo.save();
			else
			{
				std::cerr << "failed to download app info, not saving file" << std::endl;
				abort();
			}
		}
	}

	app.loadByAppID(appEngine->appToEdit);

	prepareToEdit(appEngine->appToEdit);

	buildMenu();

	float width = 14.0f;
	triangle.setFillColor(sf::Color::Red);
	triangle.setPointCount(3);
	triangle.setPoint(0, sf::Vector2f(width / 2.0f, 0.0f));
	triangle.setPoint(1, sf::Vector2f(0.0f, width));
	triangle.setPoint(2, sf::Vector2f(width, width));
	triangle.setOutlineThickness(1);
	triangle.setOutlineColor(sf::Color::Black);

	std::cout << "AppEditState ready" << std::endl;
}

void AppEditState::Cleanup()
{
	std::cout << "Cleaning up AppEditState." << std::endl;

	delete appPreview;
	delete menu;

	std::cout << "AppEditState cleaned up." << std::endl;
}

void AppEditState::Pause()
{
	std::cout << "AppEditState paused." << std::endl;
}

void AppEditState::Resume()
{
	std::cout << "AppEditState resumed." << std::endl;
}

void AppEditState::HandleEvents()
{
	sf::Event event;

	while (appEngine->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			appEngine->Quit();
		}
		else if (event.type == sf::Event::EventType::Resized)
		{
			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::FloatRect visibleArea(0.0f, 0.0f, event.size.width, event.size.height);
				appEngine->window->setView(sf::View(visibleArea));
			}
			else
			{
				if (event.size.width <= 525)
					newSize.x = 525;

				if (event.size.height <= 325)
					newSize.y = 325;

				appEngine->window->setSize(newSize);
			}
		}

		int calledbackWidget = menu->onEvent(event);
		switch (calledbackWidget)
		{
		case MenuCallbacks::DescriptionTextEntered:
		case MenuCallbacks::VersionTextEntered:
		case MenuCallbacks::AuthorTextEntered:
		case MenuCallbacks::GitHubTextEntered:
			break;
		case MenuCallbacks::IconFilePathTextEntered:
		case MenuCallbacks::DataFilePathTextEntered:
			break;
		case MenuCallbacks::CommitChanges:
		{
			std::cout << "committing changes to app" << std::endl;

			bool ignoreIcon = false, ignoreData = false, copyLocalFiles = copylocalfiles->isChecked();

			if (appName->isEmpty())
			{
				std::cerr << "not uploaded an app with no name, try again" << std::endl;
				break;
			}

			if (iconFilePath->isEmpty())
			{
				if (newApp)
				{
					std::cout << "not uploaded a new app without an icon, try again" << std::endl;
					break;
				}
				else
					ignoreIcon = true;
			}
			else
			{
				if (!iconPathValid)
				{
					std::cout << "app icon is not valid" << std::endl;
					break;
				}
				else
					app.iconpath = iconFilePath->getText();
			}

			if (dataFilePath->isEmpty())
			{
				if (newApp)
				{
					std::cout << "something is required to upload a new app" << std::endl;
					break;
				}
				else
				{
					std::cout << "no data for existing app provided... data will not be updated" << std::endl;
					ignoreData = true;
				}
			}
			else
			{
				if (!dataPathValid)
				{
					std::cout << "data path is not valid" << std::endl;
					break;
				}
				else
					app.datapath = dataFilePath->getText();
			}

			saveAppData();

			appEngine->PushState(new AppUploadState(app, ignoreIcon, ignoreData, copyLocalFiles));
			return;
		}
		case MenuCallbacks::Back:
			appEngine->PopState();
			break;
		}

		if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::Return || event.key.code == SFUI::Theme::nextWidgetKey || event.key.code == SFUI::Theme::previousWidgetKey || event.key.code == sf::Keyboard::Key::Tab)
			{
				saveAppData();
			}
		}

		if (event.type == sf::Event::EventType::TextEntered)
		{
			std::cout << "checking files" << std::endl;

			if (iconFilePath != nullptr && iconFilePath->isFocused())
			{
				if (!fs::exists(std::string(iconFilePath->getText())))
				{
					std::string s = iconFilePath->getText();
					std::cout << "icon file path is invalid (" << s << ")" << std::endl;
					iconTipString = "Icon file path is invalid.";
					iconPathValid = false;
				}
				else
				{
					if (!fs::is_directory(std::string(iconFilePath->getText())))
					{
						if (appPreview->iconTexture.loadFromFile(iconFilePath->getText()))
						{
							std::cout << "icon path is valid" << std::endl;
							iconPathValid = true;
							appPreview->icon.setTexture(&appPreview->iconTexture, true);
						}
						else
						{
							std::cerr << "icon path is valid, but the image failed to load." << std::endl;
							iconTipString = "Icon path is valid, but the image failed to load.";
							iconPathValid = false;
						}
					}
					else
					{
						std::cerr << "icon path is valid, but is a directory." << std::endl;
						iconTipString = "Icon path is valid, but is a directory.";
						iconPathValid = false;
					}
				}
			}

			if (dataFilePath != nullptr && dataFilePath->isFocused())
			{
				if (!fs::exists(std::string(dataFilePath->getText())))
				{
					dataTipString = "Data path is invalid.";
					dataPathValid = false;
				}
				else
				{
					if (!fs::is_directory(std::string(dataFilePath->getText())))
					{
						std::string str = dataFilePath->getText();

						if (str.substr(str.size() - 4, str.size()) == ".zip")
						{
							std::cout << "data path is valid" << std::endl;
							dataPathValid = true;
						}
						else
						{
							std::cerr << "file at data path exists, but is not a zip file." << std::endl;
							dataTipString = "file at data path exists, but is not a zip file.";
							dataPathValid = false;
						}
					}
					else
					{
						std::cerr << "data path exists, but is a directory." << std::endl;
						dataTipString = "data path exists, but is a directory.";
						dataPathValid = false;
					}
				}
			}
		}
	}
}

void AppEditState::Update()
{
}

void AppEditState::Draw()
{
	// FIXME: this function is a piece of shit

	appEngine->window->clear(SFUI::Theme::windowBgColor);

	if (appPreview != nullptr)
		appPreview->draw();

	appEngine->window->draw(*menu);

	if (appPreview != nullptr)
	{
		float x = 65;
		float y = -3;

		if (appName->isEmpty())
		{
			triangle.setPosition(sf::Vector2f(x + 230, appName->getAbsolutePosition().y - y));
			appEngine->window->draw(triangle);

			if (mouseIsOver(triangle))
			{
				tooltip.setString("App name is required.");
				tooltip.setPosition(sf::Vector2f(triangle.getPosition().x + 20, appName->getAbsolutePosition().y - y));

				appEngine->window->draw(tooltip);
			}
		}

		if (iconFilePath->isEmpty() || iconUploadFailed || !iconPathValid)
		{
			triangle.setPosition(sf::Vector2f(x, iconFilePath->getAbsolutePosition().y - y));
			appEngine->window->draw(triangle);

			if (mouseIsOver(triangle))
			{
				tooltip.setString(iconTipString);
				tooltip.setPosition(sf::Vector2f(triangle.getPosition().x + 20, iconFilePath->getAbsolutePosition().y - y));

				appEngine->window->draw(tooltip);
			}
		}

		if (dataFilePath->isEmpty() || dataUploadFailed || !dataPathValid)
		{
			triangle.setPosition(sf::Vector2f(x, dataFilePath->getAbsolutePosition().y - y));
			appEngine->window->draw(triangle);

			if (mouseIsOver(triangle))
			{
				tooltip.setString(dataTipString);
				tooltip.setPosition(sf::Vector2f(triangle.getPosition().x + 20, dataFilePath->getAbsolutePosition().y - y));

				appEngine->window->draw(tooltip);
			}
		}

		/*
		if (appDescription->isEmpty())
		{
			triangle.setFillColor(sf::Color::Yellow);
			triangle.setPosition(sf::Vector2f(x, appDescription->getAbsolutePosition().y - y));
			app->window->draw(triangle);
			triangle.setFillColor(sf::Color::Red);

			if (mouseIsOver(triangle))
			{
				delete toolTip;
				toolTip = new Tooltip();
				toolTip->setPosition(sf::Vector2f(x + 10, appDescription->getAbsolutePosition().y - y));
				toolTip->setString("Optional: App description not defined.", app->window->getSize().x - 10);
				app->window->draw(*toolTip);
			}
		}

		if (appVersion->isEmpty())
		{
			triangle.setFillColor(sf::Color::Yellow);
			triangle.setPosition(sf::Vector2f(x, appVersion->getAbsolutePosition().y - y));
			app->window->draw(triangle);
			triangle.setFillColor(sf::Color::Red);

			if (mouseIsOver(triangle))
			{
				delete toolTip;
				toolTip = new Tooltip();
				toolTip->setPosition(sf::Vector2f(x + 10, appVersion->getAbsolutePosition().y - y));
				toolTip->setString("Optional: App version not defined.", app->window->getSize().x - 10);
				app->window->draw(*toolTip);
			}
		}

		if (author->isEmpty())
		{
			triangle.setFillColor(sf::Color::Yellow);
			triangle.setPosition(sf::Vector2f(x, author->getAbsolutePosition().y - y));
			app->window->draw(triangle);
			triangle.setFillColor(sf::Color::Red);

			if (mouseIsOver(triangle))
			{
				delete toolTip;
				toolTip = new Tooltip();
				toolTip->setPosition(sf::Vector2f(x + 10, author->getAbsolutePosition().y - y));
				toolTip->setString("Optional: Author name is missing.", app->window->getSize().x - 10);
				app->window->draw(*toolTip);
			}
		}

		if (github->isEmpty())
		{
			triangle.setFillColor(sf::Color::Yellow);
			triangle.setPosition(sf::Vector2f(x, github->getAbsolutePosition().y - y));
			app->window->draw(triangle);
			triangle.setFillColor(sf::Color::Red);

			if (mouseIsOver(triangle))
			{
				delete toolTip;
				toolTip = new Tooltip();
				toolTip->setPosition(sf::Vector2f(x + 10, github->getAbsolutePosition().y - y));
				toolTip->setString("Optional: GitHub link is missing.", app->window->getSize().x - 10);
				app->window->draw(*toolTip);
			}
		}
		*/
	}

	appEngine->window->display();
}

void AppEditState::prepareToEdit(size_t appid)
{
	std::cout << "preparing to edit app " << appid << std::endl;

	app.appid = appid;
	const std::string appid_s = std::to_string(app.appid);

	// get an up-to-date appindex
	appEngine->updateAppIndex();

	std::string path = GBL::DIR::apps + appid_s + "/";

	if (fs::exists(path))
	{
		std::cout << "app folder exists" << std::endl;

		if (!fs::exists(path + "info.dat"))
		{
			std::cout << "app info does not exist" << std::endl;

			Download download;
			download.setInput(GBL::WEB::APPS + appid_s + "/info.dat");
			download.setOutputDir(GBL::DIR::apps + appid_s + "//");
			download.setOutputFilename("info.dat");

			int status = download.download();

			if (status == Download::Status::Ok)
			{
				std::cout << "downloaded info successfully" << std::endl;

				download.save();
			}
			else
			{
				std::cerr << "failed to download app info, creating an empty file" << std::endl;

				std::ofstream newInfo(path + "info.dat");

				if (newInfo.is_open())
				{
					newInfo << std::endl;

					newInfo.close();

					if (newInfo.bad())
					{
						std::cerr << "failed to save default config file (it may be corrupt)" << std::endl;
						abort();
					}
					else
						itemInfoParser.loadFromFile(path + "info.dat");
				}
				else
				{
					std::cerr << "failed to create empty app info file" << std::endl;
					abort();
				}
			}

			app.loadByAppID(appid);

			itemInfoParser.loadFromFile(path + "info.dat");
		}

		if (!newApp) // if it's not new, get the icon
		{
			if (fs::exists(path + "icon.png"))
			{
				std::cout << "app icon exists" << std::endl;
				app.iconpath = path + "icon.png";
				iconPathValid = true;
			}
			else
			{
				std::cout << "app icon does not exist" << std::endl;

				Download getIcon;
				getIcon.setInput(GBL::WEB::APPS + appid_s + "/icon.png");
				getIcon.setOutputDir(GBL::DIR::apps + appid_s + "/");
				getIcon.setOutputFilename("icon.png");

				int status = getIcon.download();

				if (status == Download::Status::Ok)
				{
					std::cout << "downloaded icon successfully" << std::endl;

					getIcon.save();

					app.iconpath = path + "icon.png";
					iconPathValid = true;
				}
				else
				{
					std::cerr << status << std::endl;
				}
			}
		}
	}
	else
	{
		// TODO: this might not work if the app hasn't been uploaded

		std::cerr << "app folder does not exist, we need to download it" << std::endl;

		Download downloadApp;
		downloadApp.setInput(GBL::WEB::APPS + appid_s);
		downloadApp.setOutputDir(GBL::DIR::apps + appid_s);
		downloadApp.setInputFilename("info.dat");

		int status = downloadApp.download();

		if (status == Download::Status::Ok)
		{
			std::cout << "successfully downloaded app info" << std::endl;
			app.loadByAppID(appid);
		}
		else
		{
			std::cerr << "failed to download app info" << std::endl;

			app.name = "Failed to download app info.";
			app.description = "App must not have been saved to the master app server.";

			std::ofstream newInfo(path + "info.dat");

			if (newInfo.is_open())
			{
				newInfo << std::endl;

				newInfo.close();

				if (newInfo.bad())
				{
					std::cerr << "failed to save default config file (it may be corrupt)" << std::endl;
					abort();
				}
				else
					itemInfoParser.loadFromFile(path + "info.dat");
			}
			else
			{
				std::cerr << "failed to create empty app info file" << std::endl;
				abort();
			}
		}
	}
}

void AppEditState::buildMenu()
{
	if (appPreview) 
		delete appPreview;

	appPreview = new App(appEngine->window, appEngine->window->getSize().x - 20, 75, 10, 10);
	appPreview->setAppInfo(app);
	appPreview->iconTexture.loadFromFile(GBL::DIR::apps + std::to_string(app.appid) + "/icon.png");

	if (menu) 
		delete menu;

	menu = new SFUI::Menu(*appEngine->window);
	menu->setPosition(sf::Vector2f(10, 95));

	SFUI::FormLayout* form = menu->addFormLayout();

	form->addRow("App Name: ", (appName ? appName : appName = new SFUI::InputBox), MenuCallbacks::NameTextEntered);
	appName->setText(app.name);
	form->addLabel("AppID: " + std::to_string(appPreview->info.appid));

	SFUI::HorizontalBoxLayout* iconbox = form->addHorizontalBoxLayout();
	SFUI::HorizontalBoxLayout* databox = form->addHorizontalBoxLayout();

	iconbox->addLabel("Icon Filepath: ");
	iconbox->add(iconFilePath = new SFUI::InputBox(222), MenuCallbacks::IconFilePathTextEntered);
	iconFilePath->setText(app.iconpath);
	iconbox->addButton("Browse", MenuCallbacks::IconFileBrowse);

	databox->addLabel("Data Filepath: ");
	databox->add(dataFilePath = new SFUI::InputBox(220), MenuCallbacks::DataFilePathTextEntered);
	dataFilePath->setText(app.datapath);
	databox->addButton("Browse", MenuCallbacks::DataFileBrowse);

	form->addRow("Description: ", (appDescription ? appDescription : appDescription = new SFUI::InputBox(400)), MenuCallbacks::DescriptionTextEntered);
	appDescription->setText(app.description);
	form->addRow("Version: ", (appVersion ? appVersion : appVersion = new SFUI::InputBox(80)), MenuCallbacks::VersionTextEntered);
	appVersion->setText(app.version);
	form->addRow("Author: ", (author ? author : author = new SFUI::InputBox), MenuCallbacks::AuthorTextEntered);
	author->setText(app.author);
	form->addRow("GitHub: ", (github ? github : github = new SFUI::InputBox(400)), MenuCallbacks::GitHubTextEntered);
	github->setText(app.github);

	form->addHorizontalBoxLayout();

	SFUI::HorizontalBoxLayout* saveHbox = form->addHorizontalBoxLayout();
	saveHbox->addButton("Commit Changes", MenuCallbacks::CommitChanges);
	saveHbox->addVerticalBoxLayout();
	saveHbox->addVerticalBoxLayout();
	saveHbox->add((copylocalfiles ? copylocalfiles : copylocalfiles = new SFUI::CheckBox));
	saveHbox->addLabel("Copy Icon & Data to local app folder");

	form->addButton("Back", MenuCallbacks::Back);
}

int AppEditState::registerNewApp()
{
	std::cout << "registering new appid" << std::endl;

	int newAppID = -1;

	{
		Download getNextAppID;
		getNextAppID.setInput(GBL::WEB::APPS + "index.dat");
		getNextAppID.download();

		std::cout << getNextAppID.fileBuffer << std::endl;

		// TODO: check to make sure the app name isn't already in use
		{ // put this in a block so that parser is destroyed and the index file is released
			SettingsParser parser;

			if (parser.loadFromFile(GBL::DIR::apps + "index.dat"))
			{
				parser.get("nextAppID", newAppID);
				parser.set(std::to_string(newAppID), "reservedapp");
				parser.set("nextAppID", newAppID + 1);
			}
			else
				std::cerr << "failed to open index" << std::endl;
		}

		{ // upload the modified index.dat
			sf::Ftp ftp;

			sf::Ftp::Response response = ftp.connect("files.000webhost.com", 21, sf::milliseconds(10000));
			if (!response.isOk())
			{
				std::cerr << "failed to connect to ftp (" << response.getMessage() << " (" << response.getStatus() << "))" << std::endl;
				abort();
			}

			response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
			if (!response.isOk())
			{
				std::cerr << "failed to login to ftp" << std::endl;
				abort();
			}

			response = ftp.changeDirectory("public_html");
			if (!response.isOk())
			{
				std::cerr << "failed to set ftp directory to public_html" << std::endl;
				abort();
			}

			response = ftp.upload(GBL::DIR::apps + "index.dat", GBL::WEB::APPS);
			if (!response.isOk())
			{
				std::cerr << "failed to update item index" << std::endl;
				std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;
				abort();
			}

			ftp.disconnect();
		}
	}

	std::cout << "new appid " << newAppID << std::endl;
	return newAppID;
}

void AppEditState::saveAppData()
{
	if (appPreview != nullptr)
	{
		if (appName != nullptr)
		{
			app.name = appName->getText();
			itemInfoParser.set("name", app.name);
		}

		if (appDescription != nullptr)
		{
			app.description = appDescription->getText();
			itemInfoParser.set("description", app.name);
		}

		if (appVersion != nullptr)
		{
			app.version = appVersion->getText();
			itemInfoParser.set("version", app.version);
		}

		if (author != nullptr)
		{
			app.author = author->getText();
			itemInfoParser.set("author", app.author);
		}

		if (github != nullptr)
		{
			app.github = github->getText();
			itemInfoParser.set("github", app.github);
		}

		appPreview->setAppInfo(app);
	}
}

void AppEditState::createNewAppFiles(int appid)
{
	std::string appid_s = std::to_string(appid);

	// TODO: use appid
	if (!fs::exists(std::string(GBL::DIR::apps + appid_s)))
		fs::create_directory(std::string(GBL::DIR::apps + appid_s));

	if (!fs::exists(std::string(GBL::DIR::apps + appid_s + "/info.dat")))
	{
		std::ofstream createFile(std::string(GBL::DIR::apps + appid_s + "/info.dat"), std::ios::out);

		if (createFile.is_open())
		{
			createFile.close();
			std::cout << "created info file" << std::endl;
		}
		else
		{
			std::cerr << "failed to create info.dat" << std::endl;
			abort();
		}
	}

	itemInfoParser.loadFromFile(GBL::DIR::apps + appid_s + "/info.dat");
	itemInfoParser.set("appid", appid);

	std::cout << "created new item" << std::endl;
}

bool AppEditState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(appEngine->window->mapPixelToCoords(sf::Mouse::getPosition(*appEngine->window))))
		return true;
	else
		return false;
}

bool AppEditState::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(appEngine->window->mapPixelToCoords(sf::Mouse::getPosition(*appEngine->window))))
		return true;
	else
		return false;
}
