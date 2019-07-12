#include "AppEngine.hpp"
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
	InitialEnterNameTextEntered,
	NameTextEntered,
	DescriptionTextEntered,
	VersionTextEntered,
	AuthorTextEntered,
	GitHubTextEntered,
	IconFilePathTextEntered,
	DataFilePathTextEntered,
	UploadApp,
	Back,
};

void AppUploadState::Init(AppEngine* app_)
{
	std::cout << "AppUploadState Init" << std::endl;
	app = app_;

	menu = new SFUI::Menu(*app->window);
	menu->setPosition(sf::Vector2f(10, 10));

	SFUI::FormLayout* form = menu->addFormLayout();

	form->addRow("App Name: ", appName = new SFUI::InputBox, MenuCallbacks::InitialEnterNameTextEntered);
	form->addButton("Back", MenuCallbacks::Back);

	float width = 14.0f;
	triangle.setFillColor(sf::Color::Red);
	triangle.setPointCount(3);
	triangle.setPoint(0.0f, sf::Vector2f(width / 2.0f, 0.0f));
	triangle.setPoint(1.0f, sf::Vector2f(0.0f, width));
	triangle.setPoint(2.0f, sf::Vector2f(width, width));
	triangle.setOutlineThickness(1);
	triangle.setOutlineColor(sf::Color::Black);

	toolTip = new Tooltip;

	std::cout << "AppUploadState ready" << std::endl;
}

void AppUploadState::Cleanup()
{
	std::cout << "Cleaning up AppUploadState." << std::endl;

	delete menu;

	std::cout << "AppUploadState cleaned up." << std::endl;
}

void AppUploadState::Pause()
{
	std::cout << "AppUploadState paused." << std::endl;
}

void AppUploadState::Resume()
{
	std::cout << "AppUploadState resumed." << std::endl;
}

void AppUploadState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
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
		}

		int calledbackWidget = menu->onEvent(event);
		switch (calledbackWidget)
		{
		case MenuCallbacks::InitialEnterNameTextEntered:
			addNewApp();
			break;
		case MenuCallbacks::DescriptionTextEntered:
		case MenuCallbacks::VersionTextEntered:
		case MenuCallbacks::AuthorTextEntered:
		case MenuCallbacks::GitHubTextEntered:
			break;
		case MenuCallbacks::IconFilePathTextEntered:
		case MenuCallbacks::DataFilePathTextEntered:
			break;
		case MenuCallbacks::UploadApp:
			uploadApp();
			break;
		case MenuCallbacks::Back:
			app->ChangeState(new HomeState);
			break;
		} 

		if (creatingApp && event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::Return || event.key.code == SFUI::Theme::nextWidgetKey || event.key.code == SFUI::Theme::previousWidgetKey || event.key.code == sf::Keyboard::Key::Tab)
			{
				if (appPreview != nullptr)
				{
					if (appDescription != nullptr)
					{
						appPreview->description.setString(appDescription->getText());
						appPreview->info.description = appDescription->getText();
						itemInfo.set("description", appPreview->info.description);
					}

					if (appVersion != nullptr)
					{
						appPreview->version.setString(appVersion->getText());
						appPreview->info.version = appVersion->getText();
						itemInfo.set("version", appPreview->info.version);
					}

					if (author != nullptr)
					{
						appPreview->info.author = author->getText();
						itemInfo.set("author", appPreview->info.author);
					}

					if (github != nullptr)
					{
						appPreview->info.github = github->getText();
						itemInfo.set("github", appPreview->info.github);
					}
				}
			}
		}
		
		if (creatingApp && event.type == sf::Event::EventType::TextEntered)
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
							appPreview->icon.setTexture(&appPreview->iconTexture);
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
				if ( !fs::exists(std::string(dataFilePath->getText())) )
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

void AppUploadState::Update()
{
}

void AppUploadState::Draw()
{
	// FIXME: this function is a piece of shit

	app->window->clear(SFUI::Theme::windowBgColor);

	if (appPreview != nullptr)
		appPreview->draw();

	app->window->draw(*menu);

	if (appPreview != nullptr)
	{
		float x = iconFilePath->getSize().x + 105;
		float y = -5;

		if (iconFilePath->isEmpty() || iconUploadFailed || !iconPathValid)
		{
			triangle.setPosition(sf::Vector2f(x, iconFilePath->getAbsolutePosition().y - y));
			app->window->draw(triangle);

			if (mouseIsOver(triangle))
			{
				delete toolTip;
				toolTip = new Tooltip();
				toolTip->setPosition(sf::Vector2f(x + 10, iconFilePath->getAbsolutePosition().y - y));

				toolTip->setString(iconTipString, app->window->getSize().x - 10);

				app->window->draw(*toolTip);
			}
		}

		if (dataFilePath->isEmpty() || dataUploadFailed || !dataPathValid)
		{
			triangle.setPosition(sf::Vector2f(x, dataFilePath->getAbsolutePosition().y - y));
			app->window->draw(triangle);

			if (mouseIsOver(triangle))
			{
				delete toolTip;
				toolTip = new Tooltip();
				toolTip->setPosition(sf::Vector2f(x + 10.0f, dataFilePath->getAbsolutePosition().y - y));

				toolTip->setString(dataTipString, app->window->getSize().x - 10);

				app->window->draw(*toolTip);
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

	app->window->display();
}

void AppUploadState::addNewApp()
{
	{
		Download getNextAppID;
		getNextAppID.setInput("apps/index.dat");
		getNextAppID.download();

		std::cout << getNextAppID.fileBuffer << std::endl;

		// TODO: only check names
		// currently, if an app has a name which is just numbers
		// this will deny it because it matches an appid

		// TODO: check full names
		// it will also fail if you enter "SFML" and "SFML Game Example" already exists
		if (getNextAppID.fileBuffer.find(appName->getText()) != std::string::npos)
		{
			// TODO: prompt edit of app name

			std::cerr << "app already exists" << std::endl;
			return;

			// TODO: add something for the user to see when it goes wrong
		}
		else
		{
			std::cout << "app does not already exist in index" << std::endl;

			bool completelyNewApp = true;

			for (auto& p : fs::directory_iterator(GBL::DIR::apps))
			{
				// the string of the filename
				std::string s = appName->getText();
				// the string of the path filename
				std::string pp = p.path().filename().string();
				if (pp == s)
				{
					if (fs::exists(p.path().string() + "/info.dat"))
					{
						std::cout << "app already exists in folder" << std::endl;

						MessageBox::Options mo;
						mo.text = "\"" + s + "\" was found in the apps folder, would you like to use it?\n\n selecting 'no' will erase and recreate the app.";
						mo.title = "App already exists.";
						mo.settings = { "No", "Yes" };

						MessageBox mb(mo);
						mb.runBlocking();

						if (mb.exitCode == 0) // no
						{
							try
							{
								fs::remove_all(GBL::DIR::apps + s);
								break;
							}
							catch (const std::exception& e)
							{
								std::cerr << e.what() << std::endl;
								abort();
							}
						}
						else if (mb.exitCode == 1) // yes
						{
							if (itemInfo.loadFromFile(GBL::DIR::apps + s + "/info.dat"))
							{
								std::string desc, auth, ver, git;

								appDescription = new SFUI::InputBox;
								itemInfo.get("description", desc);
								appDescription->setText(desc);

								appVersion = new SFUI::InputBox;
								itemInfo.get("version", ver);
								appVersion->setText(ver);

								author = new SFUI::InputBox;
								itemInfo.get("author", auth);
								author->setText(auth);

								github = new SFUI::InputBox;
								itemInfo.get("github", git);
								github->setText(git);

								float appid;

								completelyNewApp = false;

								std::cout << "ready" << std::endl;
								break;
							}
							else
							{
								std::cerr << "failed to load info file for reading" << std::endl;
								abort();
							}
						}
					}
					else
					{
						std::cerr << "info does not exist" << std::endl;
						break;
					}
				}
			}

			if (completelyNewApp)
				prepareNewApp();
		}
	}

	std::string appname_ = appName->getText();

	appPreview = new App(app->window, app->window->getSize().x - 20, 75, 10, 10);
	appPreview->name.setString(appname_);
	appPreview->info.name = appname_;
	itemInfo.get("appid", appPreview->info.appid);

	// add the new app to the index
	SettingsParser parser;
	parser.loadFromFile(GBL::DIR::cache + "apps/index.dat");
	parser.set(std::to_string(appPreview->info.appid), appname_);

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
		std::cerr << "failed to set ftp directory" << std::endl;
		abort();
	}

	response = ftp.upload(GBL::DIR::cache + "apps/index.dat", GBL::WEB::APPS);
	if (!response.isOk())
	{
		std::cerr << "failed to update item index" << std::endl;
		std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;
		abort();
	}

	ftp.disconnect();

	delete menu;
	menu = new SFUI::Menu(*app->window);
	menu->setPosition(sf::Vector2f(10, 95));

	SFUI::FormLayout* form = menu->addFormLayout();

	form->addLabel("App Name: " + appname_);
	form->addLabel("AppID: " + std::to_string(appPreview->info.appid));

	form->addRow("Icon Filepath: ", iconFilePath = new SFUI::InputBox, MenuCallbacks::IconFilePathTextEntered);
	form->addRow("Data Filepath: ", dataFilePath = new SFUI::InputBox, MenuCallbacks::DataFilePathTextEntered);

	form->addRow("Description: ", (appDescription ? appDescription : appDescription = new SFUI::InputBox), MenuCallbacks::DescriptionTextEntered);
	form->addRow("Version: ", (appVersion ? appVersion : appVersion = new SFUI::InputBox), MenuCallbacks::VersionTextEntered);
	form->addRow("Author: ", (author ? author : author = new SFUI::InputBox), MenuCallbacks::AuthorTextEntered);
	form->addRow("GitHub: ", (github ? github : github = new SFUI::InputBox), MenuCallbacks::GitHubTextEntered);

	form->addButton("Upload", MenuCallbacks::UploadApp);
	form->addButton("Back", MenuCallbacks::Back);

	creatingApp = true;

	return;
}

void AppUploadState::prepareNewApp()
{
	int appid = 0;

	std::string l;
	std::ifstream index(GBL::DIR::cache + "apps/index.dat", std::ios::binary);

	if (index.is_open())
	{
		// skip empty lines when we count
		index.unsetf(std::ios_base::skipws);

		// every line in the file is an app
		size_t appCount = std::count(
			std::istream_iterator<char>(index),
			std::istream_iterator<char>(),
			'\n');

		// TODO: consider putting a last line on the app index
		// lastAppID = ?
		// and then use that + 1

		std::cout << appCount + 1 << " lines; ";

		{
			// https://stackoverflow.com/questions/36820881/how-to-check-last-line-of-file-for-new-line-character-c

			std::ifstream ifs(GBL::DIR::cache + "apps/index.dat", std::ifstream::binary);

			// Read last two chars, it might also read only one last char
			std::vector<char> end_file_chars;
			for (int pos = 1; pos <= 2; ++pos)
			{
				if (!ifs.seekg(-pos, std::ios::end)) break;
				char c;
				if (ifs.get(c)) end_file_chars.insert(end_file_chars.begin(), c);
			}

			// Possible end file characters
			std::vector<std::vector<char>> endlines = {
				{ '\r', '\n' },
			{ '\n' },
			{ '\r' }
			};

			// Predicate to compare possible endline with what was found in the file.
			auto checkFn = [&](auto &endline)
			{
				// Equal compares possible endline in reverse order
				return std::equal(endline.rbegin(), endline.rend(), end_file_chars.rbegin());
			};

			// If any end file character was read and if it acually is end file character
			if (!end_file_chars.empty() && std::find_if(endlines.begin(), endlines.end(), checkFn) != endlines.end())
			{
				appCount -= 1;
				std::cout << "Found" << std::endl;
			}
			else
			{
				std::cout << "trailing newline not found" << std::endl;
			}
		}

		appCount++;
		std::cout << appCount << " appid" << std::endl;

		// I thought that appcount + 1 would be the proper thing to do
		// but for somereason appCound is always one ahead. 
		// maybe std::count does not start at zero for it's count?
		appid = appCount;
	}
	else
	{
		std::cerr << "failed to open index for reading" << std::endl;
		abort();
	}

	if (!fs::exists(std::string(GBL::DIR::apps + appName->getText())))
		fs::create_directory(std::string(GBL::DIR::apps + appName->getText()));

	if (!fs::exists(std::string(GBL::DIR::apps + appName->getText() + "/info.dat")))
	{
		std::ofstream createFile(std::string(GBL::DIR::apps + appName->getText() + "/info.dat"), std::ios::out);

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

	itemInfo.loadFromFile(GBL::DIR::apps + appName->getText() + "/info.dat");
	itemInfo.set("name", std::string(appName->getText()));
	itemInfo.set("appid", appid);

	std::cout << "created new item" << std::endl;
}

void AppUploadState::uploadApp()
{
	sf::Ftp ftp;

	sf::Ftp::Response response = ftp.connect("files.000webhost.com", 21, sf::milliseconds(10000));
	if (!response.isOk())
	{
		std::cerr << "FAILED to connect to FTP: " << response.getStatus() << ":" << response.getMessage() << std::endl;
		abort();
	}

	response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
	if (!response.isOk())
	{
		std::cerr << "FAILED to login to FTP" << std::endl;
		abort();
	}

	response = ftp.changeDirectory("public_html/apps");
	if (!response.isOk())
	{
		std::cerr << "FAILED to set ftp directory to apps" << std::endl;
		abort();
	}

	std::string appid = std::to_string(appPreview->info.appid);
	response = ftp.changeDirectory(appid);
	if (!response.isOk())
	{
		std::cerr << "FAILED to set ftp directory to app, attempting to create" << std::endl;

		response = ftp.createDirectory(appid);
		if (!response.isOk())
		{
			std::cerr << "FAILED to create directory, aborting" << std::endl;
			abort();
		}
		else
		{
			response = ftp.changeDirectory(appid);
			if (!response.isOk())
			{
				std::cerr << "FAILED to set directory, aborting" << std::endl;
				abort();
			}
		}
	}

	response = ftp.upload(GBL::DIR::apps + appPreview->info.name + "/info.dat", "./");
	if (!response.isOk())
	{
		std::cerr << "FAILED to upload info" << std::endl;
		std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;

		uploadComplete = false;
		infoUploadFailed = true;

		abort();
	}
	else
	{
		std::cout << "SUCCESS! uploaded info" << std::endl;
	}
	
	// copy the file to the local dir
	try
	{
		std::string f = iconFilePath->getText();
		std::string t = GBL::DIR::apps + appPreview->info.name + "/icon.png";

		std::cout << "from: " << f << std::endl;
		std::cout << "to: " << t << std::endl;

		fs::remove(t);
		fs::copy_file(f, t);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		abort();
	}

	response = ftp.upload(GBL::DIR::apps + appPreview->info.name + "/icon.png", "./");
	if (!response.isOk())
	{
		std::cerr << "FAILED to upload icon" << std::endl;
		std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;

		uploadComplete = false;
		iconUploadFailed = true;

		iconTipString = "Icon failed to upload.";
	}
	else
	{
		iconUploadFailed = false;
		std::cout << "SUCCESS! uploaded icon" << std::endl;
	}

	response = ftp.upload(dataFilePath->getText(), "./");
	if (!response.isOk())
	{
		std::cerr << "FAILED to upload data" << std::endl;
		std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;

		uploadComplete = false;
		dataUploadFailed = true;

		dataTipString = "Data failed to upload.";
	}
	else
	{
		dataUploadFailed = false;
		std::cout << "SUCCESS! uploaded data" << std::endl;
	}

	uploadComplete = true;

	return;
}

bool AppUploadState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

bool AppUploadState::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}
