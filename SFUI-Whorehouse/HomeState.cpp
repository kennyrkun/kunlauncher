#include "AppEngine.hpp"
#include "AppState.hpp"
#include "HomeState.hpp"
#include "AppListState.hpp"
#include "SettingsState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "Item.hpp"
#include "Section.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>

HomeState HomeState::HomeState_dontfuckwithme;

void HomeState::Init(AppEngine* app_)
{
	std::cout << "HomeState Init" << "\n";

	app = app_;

	if (!app->window->isOpen())
	{
		app->window->create(sf::VideoMode(app->settings.width, app->settings.height), "KunLauncher " + CONST::VERSION, sf::Style::Default);
		app->window->setVerticalSyncEnabled(app->settings.verticalSync);
	}

	cardScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
//	scrollbar.create();
	scrollbar.create(app->window);

	helperThread = new std::thread(&HomeState::loadApps, this);
	helperRunning = true;
	std::cout << "thread launched" << "\n";
}

void HomeState::Cleanup()
{
	delete cardScroller;

	if (helperRunning)
	{
		std::cout << "waiting on helper thread to finish" << "\n";
		helperThread->join();
	}

	sections.clear();
	//	delete app; // dont delete app because it's being used by the thing and we need it.
	//	app = nullptr;

	std::cout << "HomeState Cleanup" << "\n";
}

void HomeState::Pause()
{
	std::cout << "HomeState Pause" << "\n";
}

void HomeState::Resume()
{
	std::cout << "HomeState Resume" << "\n";
}

void HomeState::HandleEvents(sf::Event& event)
{
	if (event.type == sf::Event::EventType::Closed)
	{
		app->Quit();
	}
	else if (event.type == sf::Event::EventType::Resized)
	{
		std::cout << "new width: " << event.size.width << "\n";
		std::cout << "new height: " << event.size.height << "\n";

		sf::Vector2u newSize(event.size.width, event.size.height);

		if (newSize.x >= 525 && newSize.y >= 325)
		{
			sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
			*mainView = sf::View(visibleArea);
			app->window->setView(sf::View(visibleArea));

			cardScroller->setSize(event.size.width, event.size.height);
			cardScroller->setCenter(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2);
		}
		else
		{
			if (event.size.width <= 525)
				newSize.x = 525;

			if (event.size.height <= 325)
				newSize.y = 325;

			app->window->setSize(newSize);
		}

		updateScrollThumbSize();
	}
	else if (event.type == sf::Event::EventType::MouseWheelMoved && scrollbar.isEnabled)
	{
		if (event.mouseWheel.delta < 0) // down, or move items up
		{
			//				if (scrollbar.canScrollDown())
			//					cardScroller->move(0, scrollbar.scrollJump);

			scrollbar.moveThumbDown();

			if (scrollerBottomPosition < scrollerMaxPosition)
			{
				cardScroller->move(0, scrollbar.scrollJump);

				updateScrollLimits();

				if (scrollerBottomPosition > scrollerMaxPosition) // clamp
				{
					std::cout << "cardScroller went too far down (" << scrollerBottomPosition << ":" << scrollerMaxPosition << "), clamping..." << "\n";
					cardScroller->setCenter(cardScroller->getCenter().x, scrollerMaxPosition - cardScroller->getSize().y / 2 + 8);
					updateScrollLimits();
				}
			}
			else
			{
				std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << "\n";
			}
		}
		else if (event.mouseWheel.delta > 0) // scroll up, or move items down
		{
			//				if (scrollbar.canScrollUp())
			//					cardScroller->move(0, -scrollbar.scrollJump);

			scrollbar.moveThumbUp();

			if (scrollerTopPosition > scrollerMinPosition)
			{
				cardScroller->move(0, -scrollbar.scrollJump);

				updateScrollLimits();

				if (scrollerTopPosition < scrollerMinPosition) // clamp
				{
					std::cout << "cardScroller went too far up (" << scrollerTopPosition << ":" << scrollerMaxPosition << "), clamping..." << "\n";
					cardScroller->setCenter(cardScroller->getCenter().x, scrollerMinPosition + cardScroller->getSize().y / 2);
					updateScrollLimits();
				}
			}
			else
			{
				std::cout << "cannot scroll view up (" << scrollerTopPosition << " < " << scrollerMaxPosition << ")" << "\n";
			}
		}
	}
	else if (event.type == sf::Event::EventType::MouseButtonPressed)
	{
		if (event.key.code == sf::Mouse::Button::Left)
		{
			//sectionwdd
			for (size_t i = 0; i < sections.size(); i++)
			{
				if (mouseIsOver(sections[i]->cardShape))
				{
					std::cout << "over shape of " << sections[i]->forwardStateName << "\n";

					if (sections[i]->forwardStateName == "appListState")
					{
						app->PushState(AppListState::Instance());
					}
					else if (sections[i]->forwardStateName == "settingsState")
					{
						app->PushState(SettingsState::Instance());
					}
				}
			}
		}
	}
	else if (event.type == sf::Event::EventType::MouseButtonReleased)
	{
//			if (scrollbar.thumbDragging)
//			{
//				scrollbar.thumbDragging = false;
//				scrollbar.scrollThumb.setFillColor(CONST::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER);
//			}
	}
}

void HomeState::Update()
{
	if (helperDone && !helperRunning)
	{
		std::cout << "helper done, joining" << "\n";
		helperThread->join();

		helperDone = false;
		helperRunning = false;
	}

	for (size_t i = 0; i < threads.size(); i++)
	{
		if (threads[i].joinable())
		{
			std::cout << "joining" << "\n";

			threads[i].detach();
			threads.erase(threads.begin() + i);
		}
	}
}

void HomeState::Draw()
{
	app->window->clear(CONST::COLOR::BACKGROUND);

	//scrollable
	app->window->setView(*cardScroller);
	for (size_t i = 0; i < sections.size(); i++)
		app->window->draw(*sections[i]);

	//anchored
	app->window->setView(*mainView);
	if (scrollbar.isEnabled)
		app->window->draw(scrollbar);

	app->window->display();
}

void HomeState::loadApps() // TOOD: this.
{
	sections.clear();

	Section* appListSection = new Section("App List", "appListState", 
		(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16), 
		app->window->getSize().y, 
		(app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), 
		28);

	Section* settingsSection = new Section("Settings", "settingsState",
		(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16),
		app->window->getSize().y,
		(app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2),
		75);

	sections.push_back(appListSection);
	sections.push_back(settingsSection);

	updateScrollThumbSize();
}

void HomeState::updateScrollThumbSize()
{
	// set the scrollbar size
	float contentHeight(0);
	for (size_t i = 0; i < sections.size(); i++)
		contentHeight += sections[i]->totalHeight + 10;

	scrollbar.update(contentHeight, cardScroller->getSize().y);

	for (size_t i = 0; i < sections.size(); i++)
		sections[i]->update(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16, app->window->getSize().y, (app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), sections[i]->cardShape.getPosition().y + 43);
}

void HomeState::updateScrollLimits()
{
	scrollerTopPosition = cardScroller->getCenter().y - cardScroller->getSize().y / 2;
	scrollerBottomPosition = cardScroller->getCenter().y + cardScroller->getSize().y / 2;
	scrollerMinPosition = 0;
	scrollerMaxPosition = scrollbar.contentHeight;
}

bool HomeState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}

bool HomeState::mouseIsOver(sf::Shape &object, sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool HomeState::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}