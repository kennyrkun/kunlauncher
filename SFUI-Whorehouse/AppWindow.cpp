#include "AppWindow.hpp"

#include "App.hpp"
#include "Globals.hpp"
#include "SFUI/SFUI.hpp"

#include <iostream>
#include <SFML/Graphics.hpp>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

// FIXME: fix loading of icon
// icons for the window don't load until
// after the menu refreshes

// TODO: when opening the confirmation dialog,
// make sure the user doesn't accidentally hide 
// it by going back a different window

VisualItemInfo::VisualItemInfo()
{
	std::cout << "info ready" << std::endl;
}

VisualItemInfo::~VisualItemInfo()
{
	delete menu;

	std::cout << "vinfo destroyed" << std::endl;
}

void VisualItemInfo::updateMenu(ItemInfo info_)
{
	info = info_;
	buildMenu();
}

void VisualItemInfo::open(ItemInfo info_)
{
	updateMenu(info_);

	sf::Vector2u newSize;
	newSize.x = menu->getSize().x + 10 + image.getSize().x + 10 + 10;
	newSize.y = menu->getSize().y + 10 + 10;
	window.create(sf::VideoMode(newSize.x, newSize.y), info.name, sf::Style::Titlebar | sf::Style::Close);

	redrawRequired = true;
	Update();

	setIcon();
}

void VisualItemInfo::close()
{
	window.close();
}

void VisualItemInfo::HandleEvents()
{
	eventQueue.clear();

	sf::Event event;
	while (window.pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
			close();

		int id = menu->onEvent(event);
		switch (id)
		{
		case PANEL_CALLBACK::OpenLocal:
		{
#ifdef _WIN32
			system("start explorer.exe \"C:/Users/xxxxxxxxxx/Pictures\"");
#else
			GBL::MESSAGES::cantOpenNotWindows();
#endif
			break;
		}
		default:
			break;
		}

		eventQueue.push_back(id);

		redrawRequired = true;
	}
}

void VisualItemInfo::Update()
{
	if (redrawRequired)
	{
		window.clear(sf::Color(50, 50, 50));

		window.draw(image);
		window.draw(*menu);

		window.display();

		redrawRequired = false;
	}
}

// TODO: reimplement this
// it will require some sort of mechanism to get
// events from the main window to this subwindow
// perhaps we can figure this out with SFUI
void VisualItemInfo::focus()
{
//	SetFocus(window.getSystemHandle());

//	MessageBeep(MB_ICONINFORMATION);

//	FLASHWINFO flash;
//	flash.cbSize = sizeof(flash);
//	flash.hwnd = window.getSystemHandle();
//	flash.dwFlags = FLASHW_TIMERNOFG;
//	flash.uCount = 5u;
//	flash.dwTimeout = 5000;
//	FlashWindowEx(&flash);
}

void VisualItemInfo::buildMenu()
{
	image.setSize(sf::Vector2f(75, 75));
	image.setPosition(sf::Vector2f(10, 10));

	std::string s(GBL::DIR::appcache + std::to_string(info.appid) + "/icon.png");
	if (fs::exists(s))
		image_Tex.loadFromFile(s);
	else
		image_Tex.loadFromFile(GBL::DIR::textures + "missing_icon_icon.png");
	image.setTexture(&image_Tex, true);

	{ // no leaks here
		// don't update if we're drawing
		while (redrawRequired);

		SFUI::Menu* phdr = menu;
		if (phdr == nullptr)
			std::cerr << "YOU GET THE BIG FUCK!" << std::endl;

		menu = new SFUI::Menu(window);
		menu->setPosition(sf::Vector2f(image.getPosition().x + image.getSize().x + 10, 10));

		delete phdr;
	}

	menu->addLabel("Name: " + info.name);
	menu->addLabel("AppID: " + std::to_string(info.appid));
	menu->addLabel("Version: " + std::to_string(info.version));
	// TODO: wrap label if it's too long
	menu->addLabel("Description: " + info.description);
	menu->addLabel("Author: " + info.author);

	if (!info.github.empty())
		// TODO: make github links clickable
		menu->addLabel("GitHub: " + info.github);

	menu->addHorizontalBoxLayout();

	if (info.downloading)
		menu->addLabel("Downloading...");
	else
	{
		SFUI::HorizontalBoxLayout* hbox = menu->addHorizontalBoxLayout();

		if (!info.downloaded)
		{
			hbox->addButton("Download", PANEL_CALLBACK::Download);
		}
		else
		{
			hbox->addButton("Delete Files", PANEL_CALLBACK::Delete);
			// TODO: add this button
			// this is going to require creation of a
			// small library type thing that will work with windows
//			hbox->addButton("Browse Local Files", PANEL_CALLBACK::OpenLocal);
		}
	}

	sf::Vector2u newSize;
	newSize.x = menu->getSize().x + 10 + image.getSize().x + 10 + 10;
	newSize.y = menu->getSize().y + 10 + 10;
	window.setSize(newSize);

	sf::View newView;
	newView.setSize(sf::Vector2f(newSize.x, newSize.y));
	newView.setCenter(sf::Vector2f(newView.getSize().x / 2, newView.getSize().y / 2));
	window.setView(newView);

	setIcon();

	redrawRequired = true;

	std::cout << "menu built" << std::endl;
}

void VisualItemInfo::setIcon()
{
	std::string s(GBL::DIR::appcache + std::to_string(info.appid) + "/icon.png");
	if (fs::exists(s))
	{
		image_Icon.loadFromFile(s);
		window.setIcon(image_Icon.getSize().x, image_Icon.getSize().y, image_Icon.getPixelsPtr());
	}
	else
	{
		image_Icon.loadFromFile(GBL::DIR::textures + "missing_icon_icon.png");
		window.setIcon(image_Icon.getSize().x / 2, image_Icon.getSize().y / 2, image_Icon.getPixelsPtr());
	}
}
