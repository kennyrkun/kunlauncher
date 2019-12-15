#include "AppEngine.hpp"
#include "NewsListState.hpp"
#include "NewsEditState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "../SFUI-Whorehouse/SettingsParser.hpp"

#include <SFUI/SFUI.hpp>

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

enum MenuCallbacks
{
	BACK = -9999,
	RedownloadNewsList
};

void NewsListState::Init(AppEngine* app_)
{
	std::cout << "NewsListState Init" << std::endl;
	app = app_;

	scrollbar.create(app->window);
	scrollbar.setTrackHeight(app->window->getSize().y);
	scrollbar.setPosition(sf::Vector2f(scrollbar.getPosition().x, 0));

	refreshNewsList();

	std::cout << "creating menu" << std::endl;
	menu = new SFUI::Menu(*app->window);
	createMenu(*menu);

	std::cout << "NewsListState ready." << std::endl;
}

void NewsListState::Cleanup()
{
	std::cout << "Cleaning up NewsListState" << std::endl;

	delete menu;

	std::cout << "Cleaned up NewsListState." << std::endl;
}

void NewsListState::Pause()
{
	std::cout << "NewsListState paused" << std::endl;
}

void NewsListState::Resume()
{
	std::cout << "NewsListState resumed" << std::endl;
}

void NewsListState::HandleEvents()
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

			scrollbar.setTrackHeight(app->window->getSize().y);
			scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, 0));
			updateScrollThumbSize();

			menu->setPosition(sf::Vector2f(10, 10));
		}

		int id = menu->onEvent(event);
		switch (id)
		{
		case MenuCallbacks::RedownloadNewsList:
		{
			app->drawInformationPanel("Redownloading news...");

			refreshNewsList();

			delete menu;
			menu = new SFUI::Menu(*app->window);
			createMenu(*menu);

			break;
		}
		case MenuCallbacks::BACK:
			app->PopState();
			return;
		}

		if (id > -1)
		{
			std::cout << "editing news " << newsList[id] << std::endl;

			app->PushState(new NewsEditState(newsList[id]));
			return;
		}

		// TODO: scrollbar control class
// TODO: scroll up if widget is out of bounds
		if (scrollbar.isEnabled)
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
//						scrollbar.scrollThumb.setFillColor(GBL::color::SCROLLTHUMB_HOLD);

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
						scrollbar.scrollThumb.setFillColor(GBL::color::SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::color::SCROLLTHUMB);
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
					menu->setPosition(sf::Vector2f(10, 10));
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

					menu->setPosition(sf::Vector2f(menu->getAbsolutePosition().x, originalMenuPosition.y - ((scrollbar.scrollThumb.getPosition().y - originalThumbPosition.y) * scrollbar.scrollStep)));

//					FIXME: error zone for scrollbar (settingsstate)
//					if (sf::Mouse::getPosition(*app->window).x > scrollbar.scrollTrack.getPosition().x + 150 || 
//						sf::Mouse::getPosition(*app->window).x < scrollbar.scrollTrack.getPosition().x - 150) // error zone
//						scrollbar.scrollThumb.setPosition(scrollbar.originalPosition);
				}
				else
				{
					// TODO: find a way to not change scrollbar color every update

					/*
					if (mouseIsOver(scrollbar.scrollThumb))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
					*/
				}
			}
		}
	}
}

void NewsListState::Update()
{
}

void NewsListState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	app->window->draw(*menu);
	app->window->draw(scrollbar);

	app->window->display();
}

void NewsListState::updateScrollThumbSize()
{
	const float contentHeight = menu->getSize().y + 20; // padding * 2 = 20

	scrollbar.update(contentHeight, menu->getSize().y);

	// TODO: update scroll limits separately
	updateScrollLimits();
}

void NewsListState::updateScrollLimits()
{
	scrollerTopPosition = menu->getAbsolutePosition().y;
	scrollerBottomPosition = menu->getAbsolutePosition().y + menu->getSize().y + 10;
	scrollerMinPosition = 10; // navbar & padding
	scrollerMaxPosition = app->window->getSize().y;
}

void NewsListState::testScrollBounds()
{
	updateScrollLimits();

	if (scrollerBottomPosition < scrollerMaxPosition)
	{
		menu->setPosition(sf::Vector2f(10, -menu->getSize().y + app->window->getSize().y - 10));
		updateScrollLimits();
	}
	else if (scrollerTopPosition > scrollerMinPosition)
	{
		menu->setPosition(sf::Vector2f(10, 10));
		updateScrollLimits();
	}
}

void NewsListState::createMenu(SFUI::Menu& menu)
{
	menu.setPosition(sf::Vector2f(10, 10));

	SFUI::HorizontalBoxLayout* container = menu.addHorizontalBoxLayout();

	SFUI::VerticalBoxLayout* menuControls = container->addVerticalBoxLayout();

	menuControls->addButton("Refresh list", MenuCallbacks::RedownloadNewsList);
	menuControls->addButton("Back", MenuCallbacks::BACK);

	SFUI::VerticalBoxLayout* newsEntries = container->addVerticalBoxLayout();

	for (size_t i = 0; i < newsList.size(); i++)
		newsEntries->addButton(newsList[i], i);

	updateScrollThumbSize();
}

void NewsListState::refreshNewsList()
{
	std::cout << "populating applist" << std::endl;

	newsList.clear();

	Download getNews;
	getNews.setInput("./" + GBL::WEB::NEWS + "/news.txt");
	getNews.setOutputDir(GBL::DIR::installDir);
	getNews.setOutputFilename("/news.txt");

	if (getNews.download() == Download::Status::Ok)
		getNews.save();
	else
		std::cerr << "failed to download news" << std::endl;

	std::ifstream readIndex(GBL::DIR::installDir + "news.txt", std::ios::in | std::ios::binary);

	if (!readIndex.is_open())
	{
		std::cerr << "failed to open news.txt" << std::endl;
		abort();
	}

	std::string line; // each line of index.dat
	bool nextLineIsNewLine = true; // starts as true because the first line is a new one
	while (std::getline(readIndex, line))
	{
		if (nextLineIsNewLine)
		{
			newsList.push_back(line);
			nextLineIsNewLine = false;
		}
		else
			if (line == "-----------------------------")
				nextLineIsNewLine = true;
	}

	readIndex.close();

	std::cout << "news list reloaded" << std::endl;
}

bool NewsListState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}
