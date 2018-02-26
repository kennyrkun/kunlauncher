#include "HomeState.hpp"

#include "AppEngine.hpp"
#include "AppState.hpp"
#include "AllAppsListState.hpp"
#include "MyAppListState.hpp"
#include "SettingsState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "App.hpp"

#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

HomeState HomeState::HomeState_dontfuckwithme;

void HomeState::Init(AppEngine* app_)
{
	std::cout << "HomeState Init" << std::endl;
	app = app_;

	font.loadFromFile(GBL::DIR::fonts + "Arial.ttf");

	navbar = new Navbar(app->window);
	navbar->addSection("home");
	navbar->addSection("my apps");
	navbar->addSection("all apps");
	navbar->addSection("settings");
	navbar->sections[0]->text.setStyle(sf::Text::Style::Bold);

	viewScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());

	scrollbar.create(app->window);
	scrollbar.setBarHeight(app->window->getSize().y - 40);
	scrollbar.setPosition(sf::Vector2f(scrollbar.getPosition().x, 40));

	app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));

	app->multithreaded_process_running = true;
	app->multithreaded_process_finished = false;
	app->multithread = new std::thread(&HomeState::loadNews, this, std::ref(app->multithreaded_process_finished), 0, 10);

	std::cout << "HomeState ready" << std::endl;
}

void HomeState::Cleanup()
{
	std::cout << "Cleaning up HomeState" << std::endl;

	if (app->multithreaded_process_running)
	{
		std::cout << "waiting on helper thread to finish" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = true;
		app->multithreaded_process_running = false;
		delete app->multithread;
	}

	sections.clear();

	delete viewScroller;
	delete mainView;

	std::cout << "HomeState Cleanup" << std::endl;
}

void HomeState::Pause()
{
	std::cout << "HomeState paused" << std::endl;
}

void HomeState::Resume()
{
	std::cout << "HomeState resumed" << std::endl;
}

void HomeState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		navbar->HandleEvents(event);

		for (size_t i = 0; i < newses.size(); i++)
			newses[i]->HandleEvents(event);

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
				*mainView = sf::View(visibleArea);
				app->window->setView(sf::View(visibleArea));

				viewScroller->setSize(sf::Vector2f(event.size.width, event.size.height));
				viewScroller->setCenter(sf::Vector2f(viewScroller->getSize().x / 2, viewScroller->getSize().y / 2));

				navbar->bar.setSize(sf::Vector2f(event.size.width, 40));
				scrollbar.setBarHeight(app->window->getSize().y - 40);

				// set the scrollbar size
				updateScrollThumbSize();
			}
			else
			{
				if (event.size.width <= 525)
					newSize.x = 525;

				if (event.size.height <= 325)
					newSize.y = 325;

				app->window->setSize(newSize);
			}

			scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, 40));
			app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));
		}
		else if (event.type == sf::Event::EventType::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Button::Left)
			{
				if (!app->multithreaded_process_running)
				{
					if (mouseIsOver(navbar->sections[1]->text))
						app->PushState(MyAppListState::Instance());
					else if (mouseIsOver(navbar->sections[2]->text))
						app->PushState(AllAppsListState::Instance());
					else if (mouseIsOver(navbar->sections[3]->text))
						app->PushState(SettingsState::Instance());
				}
			}
		}
		else if (event.type == sf::Event::EventType::MouseWheelMoved)
		{
			if (event.mouseWheel.delta < 0) // down, or move apps up
			{
				scrollbar.moveThumbDown();

				if (scrollerBottomPosition < scrollerMaxPosition)
					viewScroller->move(0, scrollbar.scrollJump + 14);
				else
					std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << std::endl;

				updateScrollLimits();

				if (scrollerBottomPosition > scrollerMaxPosition) // clamp cardScroller
				{
					std::cout << "cardScroller went too far down (" << scrollerBottomPosition - scrollerMaxPosition << "), clamping..." << std::endl;
					viewScroller->setCenter(viewScroller->getCenter().x, scrollerMaxPosition - viewScroller->getSize().y / 2 + 8);
					updateScrollLimits();
				}
			}
			else if (event.mouseWheel.delta > 0) // scroll up, or move apps down
			{
				scrollbar.moveThumbUp();

				if (scrollerTopPosition > scrollerMinPosition)
					viewScroller->move(0, -scrollbar.scrollJump - 14);
				else
					std::cout << "cannot scroll view up (" << scrollerTopPosition << " > " << scrollerMaxPosition << ")" << std::endl;

				updateScrollLimits();

				if (scrollerTopPosition < scrollerMinPosition) // clamp cardScroller
				{
					std::cout << "cardScroller went too far up (" << scrollerMaxPosition - scrollerTopPosition << "), clamping..." << std::endl;
					viewScroller->setCenter(viewScroller->getCenter().x, scrollerMinPosition + viewScroller->getSize().y / 2);
					updateScrollLimits();
				}
			}
		}
	}
}

void HomeState::Update()
{
	if (app->multithreaded_process_finished)
	{
		std::cout << "helper thread finished work, joining" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = false;
		app->multithreaded_process_running = false;

		delete app->multithread;
	}

	navbar->Update();

	for (size_t i = 0; i < newses.size(); i++)
		newses[i]->Update();
}

void HomeState::Draw()
{
	app->window->clear(GBL::COLOR::BACKGROUND);

	app->window->setView(*viewScroller);
	for (size_t i = 0; i < newses.size(); i++)
		newses[i]->Draw();

	app->window->setView(*mainView);
	app->window->draw(scrollbar);
	navbar->Draw();

	if (app->multithreaded_process_running)
		app->ShowMultiThreadedIndicator();

	app->window->display();
}

void HomeState::loadNews(bool &finishedIdicator, int start, int maxLoad)
{
	finishedIdicator = false;

	std::cout << "loading AllApps" << std::endl;

	newses.clear();

	std::cout << std::endl; // for a line break

	std::string line; // each line of index.dat;

	std::ifstream readIndex(GBL::DIR::installDir + "news.txt", std::ios::in);

	bool newNews(false);

	int loopi(0);
	while (std::getline(readIndex, line))
	{
		std::cout << loopi << " - " << line << std::endl;

		if (line == "-----------------------------")
		{
			std::cout << "end of current news" << std::endl;

			newNews = true;
			loopi += 1;
			continue;
		}
		else
		{
			newNews = false;
		}

		std::string title(line), text;

		bool exit(false);
		while (!exit)
		{
			std::string newline;
			std::getline(readIndex, newline);

			if (newline != "-----------------------------")
			{
				if (newline.empty())
					break;

				text.append(newline + "\n");
			}
			else
			{
				text.erase(text.length() - 1, text.length());

				std::cout << "==========news=============" << std::endl;
				std::cout << title << std::endl;
				std::cout << text << std::endl;

				break;
			}
		}

		News* newNews = new News(
			title,
			text,
			app->window);

		if (newses.empty())
			newNews->setPosition(sf::Vector2f(10, 45));
		else
			newNews->setPosition(sf::Vector2f(10, newses.back()->getPosition().y + newses.back()->getLocalHeight() + 25));

		newses.push_back(newNews);

		updateScrollThumbSize();
		loopi += 1;

		if (loopi == maxLoad)
			break; // only load this many (default 10);
	}

	std::cout << newses.size() << std::endl;

	readIndex.close();

	finishedIdicator = true;
}

void HomeState::updateScrollThumbSize()
{
	float contentHeight(25);
	for (size_t i = 0; i < newses.size(); i++)
		contentHeight += newses[i]->getLocalHeight() + 25;

	scrollbar.update(contentHeight, viewScroller->getSize().y);

	updateScrollLimits();
}

void HomeState::updateScrollLimits()
{
	scrollerTopPosition = viewScroller->getCenter().y - viewScroller->getSize().y / 2;
	scrollerBottomPosition = viewScroller->getCenter().y + viewScroller->getSize().y / 2;
	scrollerMinPosition = 0;
	scrollerMaxPosition = scrollbar.contentHeight;
}

bool HomeState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
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
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}
