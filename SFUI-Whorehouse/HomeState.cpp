#include "AppEngine.hpp"
#include "AppState.hpp"
#include "HomeState.hpp"
#include "AppListState.hpp"

#include "Download.hpp"
#include "Modal.hpp"
#include "Item.hpp"
#include "Section.hpp"
#include "Globals.hpp"

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

	cardScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());

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
	printf("HomeState Pause\n");
}

void HomeState::Resume()
{
	std::cout << "HomeState Resume" << "\n";
}

void HomeState::HandleEvents()
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

				// set the scrollbar size
				updateScrollThumb();

				for (size_t i = 0; i < sections.size(); i++)
					sections[i]->update();
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
		else if (event.type == sf::Event::EventType::MouseWheelMoved) // thanks sfconsole
		{
//			std::cout << "center x: " << app->window->getView().getCenter().x << "\n";
//			std::cout << "center y: " << app->window->getView().getCenter().y << "\n";
//			std::cout << "size x: " << app->window->getView().getSize().x << "\n";
//			std::cout << "size y: " << app->window->getView().getSize().y << "\n";

			if (event.mouseWheel.delta < 0) // up
			{
//				if ((cardScroller->getCenter().y - cardScroller->getSize().y) < scrollbar.scrollJumpMultiplier) // bottom of the thing
//				{
				cardScroller->move(0, scrollbar.scrollJump);
				scrollbar.moveThumbUp();
//				}
			}
			else if (event.mouseWheel.delta > 0) // scroll down
			{
				if ((cardScroller->getCenter().y - cardScroller->getSize().y / 2) > scrollbar.scrollJumpMultiplier) // top of the thing
				{
					cardScroller->move(0, -scrollbar.scrollJump);
					scrollbar.moveThumbDown();
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
						std::cout << "over shape of " << sections[i]->forwardStateName << std::endl;

						if (sections[i]->forwardStateName == "appListState")
							app->ChangeState(AppListState::Instance());
						else if (sections[i]->forwardStateName == "settingsState")
							// do nothing
							continue;
						else
							std::cout << "over nothing" << std::endl;
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
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::R)
			{
				if (helperRunning)
				{
					std::cout << "refreshing applist" << "\n";

					sections.clear();

					helperThread = new std::thread(&HomeState::loadApps, this);
					helperDone = false;
					helperRunning = true;

					cardScroller->setCenter(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2);
					scrollbar.update(scrollbar.contentHeight, cardScroller->getSize().y);
				}
				else
				{
					std::cout << "helper is running, not reloading." << "\n";
				}
			}
		}
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
		sections[i]->draw();

	//anchored
	//	app->window->setView(app->window->getDefaultView());
	// HACK: don't do this over and over. why does it even change when we scroll? I don't understand!
	app->window->setView(*mainView);
	scrollbar.draw();

	app->window->display();
}

void HomeState::loadApps() // TOOD: this.
{
	Section* appListSection = new Section("App List", "appListState", app->window, 28, true);
	sections.push_back(appListSection);

//	Section* settingsSection = new Section("settings", "null", app->window, sections.back()->cardShape.getPosition().y + 48, false);
//	sections.push_back(settingsSection);
}

void HomeState::updateScrollThumb()
{
	// set the scrollbar size
	float contentHeight(0);

	for (size_t i = 0; i < sections.size(); i++)
		contentHeight += sections[i]->totalHeight;

	scrollbar.update(contentHeight, cardScroller->getSize().y);
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