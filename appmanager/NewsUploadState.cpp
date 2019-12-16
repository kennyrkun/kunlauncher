#include "AppEngine.hpp"
#include "NewsUploadState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "../SFUI-Whorehouse/SettingsParser.hpp"

#include <SFUI/SFUI.hpp>

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

void NewsUploadState::Init(AppEngine* app_)
{
	std::cout << "NewsUploadState Init" << std::endl;
	app = app_;

	uploadNewsFile();

	std::cout << "NewsUploadState ready." << std::endl;
}

void NewsUploadState::Cleanup()
{
	std::cout << "Cleaning up NewsUploadState" << std::endl;

	std::cout << "Cleaned up NewsUploadState." << std::endl;
}

void NewsUploadState::Pause()
{
	std::cout << "NewsUploadState paused" << std::endl;
}

void NewsUploadState::Resume()
{
	std::cout << "NewsUploadState resumed" << std::endl;
}

void NewsUploadState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			std::cout << "quitting" << std::endl;

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
				sf::FloatRect visibleArea(0.0f, 0.0f, event.size.width, event.size.height);
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

void NewsUploadState::Update()
{
}

void NewsUploadState::Draw()
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

void NewsUploadState::uploadNewsFile()
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

	response = ftp.changeDirectory("public_html/news");
	if (!response.isOk())
	{
		std::cerr << "FAILED to set ftp directory to apps" << std::endl;
		abort();
	}

	messages.push_back("set upload directory");

	response = ftp.upload(GBL::DIR::installDir + "/news.txt", "./", sf::Ftp::TransferMode::Binary);
	if (!response.isOk())
	{
		std::cerr << "FAILED to upload news" << std::endl;
		std::cerr << response.getStatus() << ": " << response.getMessage() << std::endl;

		uploadComplete = false;

		abort();
	}
	else
	{
		std::cout << "SUCCESS! uploaded info" << std::endl;
		messages.push_back("SUCCESS -> info");
	}

	std::cout << "updating app index" << std::endl;
	messages.push_back("updating app index");

	std::cout << "upload complete" << std::endl;
	messages.push_back("upload complete");
	messages.push_back("");
	messages.push_back("press escape to return to app editor");

	uploadComplete = true;

	return;
}
