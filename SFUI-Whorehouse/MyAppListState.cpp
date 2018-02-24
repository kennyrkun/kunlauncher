#include "AppEngine.hpp"
#include "AppState.hpp"
#include "MyAppListState.hpp"
#include "AllAppsListState.hpp"
#include "HomeState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "App.hpp"
#include "SettingsParser.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

MyAppListState MyAppListState::MyAppListState_dontfuckwithme;

namespace fs = std::experimental::filesystem;

void MyAppListState::Init(AppEngine* app_)
{
	std::cout << "MyAppListState Init" << std::endl;

	app = app_;

	cardScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	scrollbar.create(app->window);

	app->multithreaded_process_finished = false;
	app->multithreaded_process_running = true;
	app->multithread = new std::thread(&MyAppListState::loadApps, this, std::ref(app->multithreaded_process_finished));

	std::cout << "thread launched" << std::endl;
}

void MyAppListState::Cleanup()
{
	std::cout << "Cleaning up MyAppListState." << std::endl;

	delete cardScroller;
	delete mainView;

	if (app->multithreaded_process_running)
	{
		std::cout << "waiting on helper thread to finish" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = true;
		app->multithreaded_process_running = false;
		delete app->multithread;
	}

	apps.clear();

	//	delete app; // dont delete app because it's being used by the thing and we need it.
	//	app = nullptr;

	std::cout << "MyAppListState cleaned up." << std::endl;
}

void MyAppListState::Pause()
{
	std::cout << "MyAppListState Pause" << std::endl;
}

void MyAppListState::Resume()
{
	std::cout << "MyAppListState Resume" << std::endl;
}

void MyAppListState::HandleEvents()
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
				*mainView = sf::View(visibleArea);
				app->window->setView(sf::View(visibleArea));

				cardScroller->setSize(sf::Vector2f(event.size.width, event.size.height));
				cardScroller->setCenter(sf::Vector2f(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2));

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
		}
		else if (event.type == sf::Event::EventType::MouseWheelMoved && scrollbar.isEnabled)
		{
			if (event.mouseWheel.delta < 0) // down, or move apps up
			{
				scrollbar.moveThumbDown();

				if (scrollerBottomPosition < scrollerMaxPosition)
					cardScroller->move(0, scrollbar.scrollJump);
				else
					std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << std::endl;

				updateScrollLimits();

				if (scrollerBottomPosition > scrollerMaxPosition) // clamp cardScroller
				{
					std::cout << "cardScroller went too far down (" << scrollerBottomPosition - scrollerMaxPosition << "), clamping..." << std::endl;
					cardScroller->setCenter(cardScroller->getCenter().x, scrollerMaxPosition - cardScroller->getSize().y / 2 + 8);
					updateScrollLimits();
				}
			}
			else if (event.mouseWheel.delta > 0) // scroll up, or move apps down
			{
				scrollbar.moveThumbUp();

				if (scrollerTopPosition > scrollerMinPosition)
					cardScroller->move(0, -scrollbar.scrollJump);
				else
					std::cout << "cannot scroll view up (" << scrollerTopPosition << " > " << scrollerMaxPosition << ")" << std::endl;

				updateScrollLimits();

				if (scrollerTopPosition < scrollerMinPosition) // clamp cardScroller
				{
					std::cout << "cardScroller went too far up (" << scrollerMaxPosition - scrollerTopPosition << "), clamping..." << std::endl;
					cardScroller->setCenter(cardScroller->getCenter().x, scrollerMinPosition + cardScroller->getSize().y / 2);
					updateScrollLimits();
				}
			}
			/* SCROLL CODE
			else if (!scrollbar.thumbDragging)
				if (mouseIsOver(scrollbar.scrollThumb, mainView))
					scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER);
				else
					if (scrollbar.scrollThumb.getFillColor() == GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER)
						scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB);
			*/
		}
		else if (event.type == sf::Event::EventType::MouseButtonPressed && !app->multithreaded_process_running)
		{
			/*
			if (mouseIsOver(scrollbar.scrollThumb, mainView))
			{
				scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOLD);
				scrollbar.originalPosition = scrollbar.scrollThumb.getPosition();
				scrollbar.dragOffset = sf::Vector2i(scrollbar.scrollThumb.getPosition()) - sf::Mouse::getPosition(*app->window);
				scrollbar.thumbDragging = true;
				std::cout << "started dragging the scrollbar" << std::endl;
			}
			*/
		} 
		else if (event.type == sf::Event::EventType::MouseButtonReleased && !app->multithreaded_process_running)
		{
			if (event.mouseButton.button == sf::Mouse::Button::Left && !scrollbar.thumbDragging)
			{
				bool clicked(false);

				//links
				for (size_t i = 0; i < apps.size(); i++)
				{
					if (apps[i]->downloaded)
					{
						if (mouseIsOver(apps[i]->removeButton))
						{
							MessageBox::Options modOptions;
							modOptions.title = "Confirm Deletion";
							modOptions.text = "Delete \"" + apps[i]->info.name + "\"?";
							std::vector<std::string> modaloptions = { "No", "Yes" };
							modOptions.settings = modaloptions;
							
							MessageBox confirmDelete(modOptions);
							confirmDelete.runBlocking();

							if (confirmDelete.returnCode == 0)
							{
								std::cout << "answer no" << std::endl;
							}
							else if (confirmDelete.returnCode == 1)
							{
								std::cout << "answer yes" << std::endl;

								threads.push_back(std::thread(&App::deleteFiles, apps[i]));

								clicked = true;
							}
						}
						else if (mouseIsOver(apps[i]->redownloadButton))
						{
							std::cout << "redownload button pressed" << std::endl;

							threads.push_back(std::thread(&App::updateItem, apps[i]));

							clicked = true;
						}
						else if (mouseIsOver(apps[i]->launchButton))
						{
							std::cout << "launch button pressed" << std::endl;

							apps[i]->openItem();

							clicked = true;
						}
					}
					else
					{
						if (mouseIsOver(apps[i]->downloadButton))
						{
							std::cout << "download button pressed" << std::endl;

							threads.push_back(std::thread(&App::download, apps[i]));

							clicked = true;
						}
					}

					if (clicked)
						break;
				}
			}
			else if (event.key.code == sf::Mouse::Button::Right)
			{
				app->PopState();
			}

			/* SCROLL CODE
			if (scrollbar.thumbDragging)
			{
				if (!mouseIsOver(scrollbar.scrollThumb))
					scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB);
				else
					scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER);

				scrollbar.thumbDragging = false;
				std::cout << "stopped dragging scrollbar" << std::endl;
			}
			*/
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::R && !app->multithreaded_process_running)
			{
				std::cout << "refreshing applist" << std::endl;

				apps.clear();

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) // redownload apps list
				{
					try
					{
						fs::remove(GBL::DIR::apps + "//index.dat");
					}
					catch (const std::exception& e)
					{
						std::cout << "failed to remove item index:" << std::endl;
						std::cout << e.what() << std::endl;
					}

					Download getNewIndex;
					getNewIndex.setInput(".//" + GBL::WEB::APPS + "//index.dat");
					getNewIndex.setOutputDir(GBL::DIR::apps);
					getNewIndex.setOutputFilename("//index.dat");
					getNewIndex.download();
					getNewIndex.save();
				}

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) // nuke everything and redownload
				{
					try
					{
						fs::remove_all(GBL::DIR::apps);
						fs::create_directory(GBL::DIR::apps);

						Download getNewIndex;
						getNewIndex.setInput(".//" + GBL::WEB::APPS + "//index.dat");
						getNewIndex.setOutputDir(GBL::DIR::apps);
						getNewIndex.setOutputFilename("//index.dat");
						getNewIndex.download();
						getNewIndex.save();
					}
					catch (const std::exception& e)
					{
						std::cout << "failed to remove apps folder: " << std::endl;
						std::cout << e.what() << std::endl;
					}
				}

				app->multithreaded_process_finished = false;
				app->multithreaded_process_running = true;
				app->multithread= new std::thread(&MyAppListState::loadApps, this, std::ref(app->multithreaded_process_finished));

				cardScroller->setCenter(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2);
			}
			else if (event.key.code == sf::Keyboard::Key::Escape)
			{
				if (!app->multithreaded_process_running)
				{
					app->ChangeState(HomeState::Instance());
				}
				else
				{
					std::cout << "helper is running, not switching states" << std::endl;
				}
			}
		}
		/* SCROLL CODE
		else if (event.type == sf::Event::EventType::MouseMoved)
		{
			if (scrollbar.thumbDragging)
			{
				scrollbar.scrollThumb.setPosition(sf::Vector2f(scrollbar.scrollbar.getPosition().x, sf::Mouse::getPosition(*app->window).y + scrollbar.dragOffset.y));
				cardScroller->setCenter(sf::Vector2f(cardScroller->getCenter().x,
					scrollbar.scrollThumb.getPosition().y + (cardScroller->getSize().y / 2)));

				if (sf::Mouse::getPosition(*app->window).x > scrollbar.scrollbar.getPosition().x + 150 || sf::Mouse::getPosition(*app->window).x < scrollbar.scrollbar.getPosition().x - 150) // error zone
					scrollbar.scrollThumb.setPosition(scrollbar.originalPosition);
			}
			else if (!scrollbar.thumbDragging)
				if (mouseIsOver(scrollbar.scrollThumb, mainView))
					scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER);
				else
					if (scrollbar.scrollThumb.getFillColor() == GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER)
						scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB);
		}
		*/
	}
}

void MyAppListState::Update()
{
	if (app->multithreaded_process_finished)
	{
		std::cout << "helper thread finished work, joining" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = false;
		app->multithreaded_process_running = false;

		delete app->multithread;
	}
}

void MyAppListState::Draw()
{
	app->window->clear(GBL::COLOR::BACKGROUND);

	//scrollable
	app->window->setView(*cardScroller);
	for (size_t i = 0; i < apps.size(); i++)
		apps[i]->draw();

	//anchored
	app->window->setView(*mainView);

	if (scrollbar.isEnabled)
		app->window->draw(scrollbar);

	if (app->multithreaded_process_running)
		app->ShowMultiThreadedIndicator();

	app->window->display();
}

App* createApp(std::string itemName_, sf::RenderWindow* target_window, float xSize, float ySize, float xPos, float yPos)
{
	App* app = new App(itemName_, target_window, xSize, ySize, xPos, yPos);
	return app;
}

void MyAppListState::loadApps(bool &finishedIndicator)
{
	finishedIndicator = false;

	std::cout << "loading MyApps" << std::endl;

	sf::Clock appLoadTime;

	apps.clear();
	updateScrollThumbSize();
	std::cout << std::endl; // for a line break

	std::vector<std::string> appList = get_directories(GBL::DIR::apps);

	for (size_t i = 0; i < appList.size(); i++)
	{
		std::cout << i << " - " << appList[i] << " - " << i << std::endl;

		App* newItem;

		if (apps.empty())
			newItem = createApp(appList[i], app->window,
			(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16),
				app->window->getSize().y, // I'm not sure what this is for?????
				(app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), // mid-window, excluding scrollbar size
				(75 / 2) + 10);
		else
			newItem = createApp(appList[i], app->window,
			(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16.0f),
				app->window->getSize().y, // I'm not sure what this is for?????
				(app->window->getSize().x / 2.0f) - (scrollbar.scrollbar.getSize().x / 2.0f), // the middle of the window (exluding the size of the scrollbar)
				apps.back()->cardShape.getPosition().y + apps.back()->totalHeight + 10.0f /* PADDING */);

		apps.push_back(newItem);
		std::cout << std::endl;

		//TODO: apps with updates go to top
//		if (apps.back()->updateIsAvailable)
//		{
//			App *updateItem = apps.back; // temporary copy
//			apps.pop_back(); // remove from applist
//			apps.insert(apps.begin(), updateItem); // put in front of applist
//		}

		updateScrollThumbSize();
	}

	std::cout << "finished loading apps" << " (" << apps.size() << " apps) in " << appLoadTime.getElapsedTime().asSeconds() << " seconds" << std::endl;

	app->window->requestFocus();

	finishedIndicator = true;
}

void MyAppListState::updateScrollThumbSize()
{
	// set the scrollbar size

	float contentHeight(0);
	for (size_t i = 0; i < apps.size(); i++)
		contentHeight += apps[i]->totalHeight + 10;

	scrollbar.update(contentHeight, cardScroller->getSize().y);

	for (size_t i = 0; i < apps.size(); i++)
		apps[i]->updateSize(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16, app->window->getSize().y, (app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), apps[i]->cardShape.getPosition().y + 43);

	updateScrollLimits();
}

void MyAppListState::updateScrollLimits()
{
	scrollerTopPosition = cardScroller->getCenter().y - cardScroller->getSize().y / 2;
	scrollerBottomPosition = cardScroller->getCenter().y + cardScroller->getSize().y / 2;
	scrollerMinPosition = 0;
	scrollerMaxPosition = scrollbar.contentHeight;
}

bool MyAppListState::mouseIsOver(const sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}

bool MyAppListState::mouseIsOver(const sf::Shape &object, const sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool MyAppListState::mouseIsOver(const sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}

std::vector<std::string> MyAppListState::get_directories(const std::string& s)
{
	std::vector<std::string> r;
	for (auto& p : fs::directory_iterator(s))
		if (p.status().type() == fs::file_type::directory)
			r.push_back( p.path().string().substr( GBL::DIR::apps.size(), p.path().string().size() ) );
	return r;
}
