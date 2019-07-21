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

AppUploadState::AppUploadState(AppInfo app, bool ignoreIcon, bool ignoreData, bool copylocalfiles/*, bool newApp*/)
: appToUpload(app)
, ignoreIcon(ignoreIcon)
, ignoreData(ignoreData)
, copylocalfiles(copylocalfiles)
//, updateAppName(newApp)
{
}

void AppUploadState::Init(AppEngine* app_)
{
	std::cout << "AppUploadState Init" << std::endl;
	app = app_;

	app->drawInformationPanel("Uploading app");

	uploadApp();

	std::cout << "AppUploadState ready" << std::endl;
}

void AppUploadState::Cleanup()
{
	std::cout << "Cleaning up AppUploadState." << std::endl;

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
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::Escape)
			{
				app->PopState();
				return;
			}
		}
	}
}

void AppUploadState::Update()
{
}

void AppUploadState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	sf::Text t;
	t.setFont(SFUI::Theme::getFont());
	t.setCharacterSize(14);

	std::string string;

	for (size_t i = 0; i < messages.size(); i++)
		string += messages[i] + "\n";

	t.setString(string);

	app->window->draw(t);

	app->window->display();
}

void AppUploadState::uploadApp()
{
	messages.push_back("uploading app");

	sf::Ftp ftp;

	sf::Ftp::Response response = ftp.connect("files.000webhost.com", 21, sf::milliseconds(10000));
	if (!response.isOk())
	{
		std::cerr << "FAILED to connect to FTP: " << response.getStatus() << ":" << response.getMessage() << std::endl;
		abort();
	}

	messages.push_back("connected to ftp");

	response = ftp.login("kunlauncher", "9fH^!U2=Ys=+XJYq");
	if (!response.isOk())
	{
		std::cerr << "FAILED to login to FTP" << std::endl;
		abort();
	}

	messages.push_back("logged in to ftp");

	response = ftp.changeDirectory("public_html/apps");
	if (!response.isOk())
	{
		std::cerr << "FAILED to set ftp directory to apps" << std::endl;
		abort();
	}

	messages.push_back("set upload directory");

	std::string appid_s = std::to_string(appToUpload.appid);

	response = ftp.changeDirectory(appid_s);
	if (!response.isOk())
	{
		std::cerr << "FAILED to set ftp directory to app, attempting to create" << std::endl;

		response = ftp.createDirectory(appid_s);
		if (!response.isOk())
		{
			std::cerr << "FAILED to create directory, aborting" << std::endl;
			abort();
		}
		else
		{
			response = ftp.changeDirectory(appid_s);
			if (!response.isOk())
			{
				std::cerr << "FAILED to set directory, aborting" << std::endl;
				abort();
			}
		}
	}

	messages.push_back("set upload directory to app directory");

	response = ftp.upload(GBL::DIR::apps + appid_s + "/info.dat", "./");
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
		messages.push_back("SUCCESS -> info");
	}

	if (!ignoreIcon)
	{
		// we have to copy it and rename it to make sure it's icon.png
		try
		{
			std::string filename = appToUpload.iconpath;
			filename = filename.substr(filename.rfind('/') + 1, filename.length());
			std::cout << "filename is probably: " << filename << std::endl;

			if (fs::exists(GBL::DIR::cache + filename))
			{
				std::cout << "file already exists in cache, removing" << std::endl;
				fs::remove(GBL::DIR::cache + filename);
			}

			fs::copy_file(appToUpload.iconpath, GBL::DIR::cache + "icon.png");

//			fs::rename(GBL::DIR::cache + filename, GBL::DIR::cache + "icon.png");
		}
		catch (const std::exception& e)
		{
			std::cerr << "failed to copy or rename icon file:" << std::endl;
			std::cerr << e.what() << std::endl;
			messages.push_back("failed to copy or rename icon file");
		}

		response = ftp.upload(GBL::DIR::cache + "icon.png", "./", sf::Ftp::Binary);
		if (!response.isOk())
		{
			std::cerr << "FAILED to upload icon" << std::endl;
			std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;
			messages.push_back("FAILED -> icon");

			uploadComplete = false;
			iconUploadFailed = true;
		}
		else
		{
			iconUploadFailed = false;
			std::cout << "SUCCESS! uploaded icon" << std::endl;
			messages.push_back("SUCCESS -> icon");
		}
	}
	else
	{
		std::cout << "ignoring icon" << std::endl;
		messages.push_back("IGNORING -> icon");
	}

	if (!ignoreData)
	{
		response = ftp.upload(appToUpload.datapath, "./", sf::Ftp::Binary);
		if (!response.isOk())
		{
			std::cerr << "FAILED to upload data" << std::endl;
			std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;

			uploadComplete = false;
			dataUploadFailed = true;
		}
		else
		{
			dataUploadFailed = false;
			std::cout << "SUCCESS! uploaded data" << std::endl;
			messages.push_back("SUCCESS -> data");
		}
	}
	else
	{
		std::cout << "ignoring data" << std::endl;
		messages.push_back("IGNORING -> data");
	}

	if (copylocalfiles)
	{
		messages.push_back("copying local files");

		// copy the file to the local dir
		try
		{
			std::cout << "copying icon to local directory" << std::endl;

			std::string from = appToUpload.iconpath;
			std::string to = GBL::DIR::apps + appid_s + "/icon.png";

			if (from != to)
			{
				std::cout << "from: " << from << std::endl;
				std::cout << "to: " << to << std::endl;

				fs::remove(to);
				fs::copy_file(from, to);
			}
			else
			{
				std::cout << "skip copy icon to local app folder (because it's already there)" << std::endl;
				messages.push_back("skipping copy icon");
			}
		}
		catch (const std::exception& e)
		{
			messages.push_back("failed to copy icon");

			std::cerr << e.what() << std::endl;
			abort();
		}

		try
		{
			std::cout << "copying data to local directory" << std::endl;

			std::string from = appToUpload.datapath;
			std::string to = GBL::DIR::apps + appid_s + "/release.zip";

			if (from != to)
			{
				std::cout << "from: " << from << std::endl;
				std::cout << "to: " << to << std::endl;

				fs::remove(to);
				fs::copy_file(from, to);
			}
			else
			{
				std::cout << "skip copy data to local app folder (because it's already there)" << std::endl;
				messages.push_back("skipping copy data");
			}
		}
		catch (const std::exception& e)
		{
			messages.push_back("failed to copy data");

			std::cerr << e.what() << std::endl;
			abort();
		}
	}
	else
	{
		std::cout << "not copying to local files" << std::endl;
		messages.push_back("skipping copy of local files");
	}

	std::cout << "updating app index" << std::endl;
	messages.push_back("updating app index");

	response = ftp.changeDirectory("../");
	if (!response.isOk())
	{
		std::cerr << "failed to move out of app directory" << std::endl;
		std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;
		messages.push_back("failed to move out of app directory");

		uploadComplete = false;
	}
	else
	{
		std::cout << "moved out of app directory" << std::endl;
		messages.push_back("moved out of app directory");
	}

	std::cout << "uploading new app index" << std::endl;
	messages.push_back("uploading new app index");

	response = ftp.upload(GBL::DIR::apps + "index.dat", "./", sf::Ftp::Binary);
	if (!response.isOk())
	{
		std::cerr << "FAILED to upload index" << std::endl;
		std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;
		messages.push_back("failed to upload index");

		uploadComplete = false;
	}
	else
	{
		std::cout << "uploaded new app index" << std::endl;
		messages.push_back("uploaded new app index");
	}

	std::cout << "upload complete" << std::endl;
	messages.push_back("upload complete");
	messages.push_back("");
	messages.push_back("press escape to return to app editor");

	uploadComplete = true;

	return;
}
