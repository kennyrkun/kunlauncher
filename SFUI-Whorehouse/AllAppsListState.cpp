#include "AppEngine.hpp"
#include "AllAppsListState.hpp"
#include "HomeState.hpp"
#include "MyAppListState.hpp"
#include "SettingsState.hpp" // TODO: find a better way to include these for the navbar

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "StoreApp.hpp"
#include "SettingsParser.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

// TODO: figure out some way to have all app data in one spot
// so we don't have duplicates between apps and appcache

void AllAppsListState::Init(AppEngine* app_)
{
	std::cout << "AllAppsListState Init" << std::endl;

	app = app_;

	navbar = new Navbar(app->window);
	navbar->addSection("home");
	navbar->addSection("my apps");
	navbar->addSection("all apps").setStyle(sf::Text::Style::Bold);
	navbar->addSection("settings");

	viewScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());

	scrollbar.create(app->window);
	scrollbar.setTrackHeight(app->window->getSize().y - navbar->bar.getSize().y);
	scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, 0.0f + navbar->bar.getSize().y));

	app->multithreaded_process_finished = false;
	app->multithreaded_process_running = true;
	app->multithread = new std::thread(&AllAppsListState::loadApps, this, std::ref(app->multithreaded_process_finished));

	app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20.0f, app->window->getSize().y - 20.0f));
}

void AllAppsListState::Cleanup()
{
	std::cout << "Cleaning up AllAppsListState." << std::endl;

	if (app->multithreaded_process_running)
	{
		std::cout << "waiting on helper thread to finish" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = true;
		app->multithreaded_process_running = false;
		delete app->multithread;
	} 

	sf::Text downloading;
	downloading.setFont(*GBL::theme.getFont("Arial.ttf"));
	downloading.setCharacterSize(16);
	downloading.setPosition(sf::Vector2f(45.0f, app->window->getSize().y - 30.0f));

	// TODO: put infoPanel back into private
	// TODO: remove this
	// TODO: rewrite infoPanel interaction system
	for (size_t i = 0; i < apps.size(); i++)
		if (apps[i]->infoPanel.isOpen())
			apps[i]->infoPanel.close();

	while (!GBL::threadManager.empty())
	{
		downloading.setString("Some apps are still downloading... (" + std::to_string(GBL::threadManager.tasks()) + ")");

		app->window->clear(GBL::theme.palatte.TERTIARY);
		app->window->draw(downloading);
		app->ShowMultiThreadedIndicator();
		app->window->display();

		GBL::threadManager.update();
	}

	std::cout << "everything is done" << std::endl;

	// TODO: instead of clearing the applist, let's keep it as a cache,
	// and just check to see if the remote has changed, and if it has, then reload it
	// in the future, we can add apps where they are in numerical order
	for (size_t i = 0; i < apps.size(); i++)
		delete apps[i];
	apps.clear();

	delete mainView;
	delete viewScroller;
	delete navbar;

	std::cout << "AllAppsListState cleaned up." << std::endl;
}

void AllAppsListState::Pause()
{
	std::cout << "AllAppsListState Pause" << std::endl;
}

void AllAppsListState::Resume()
{
	std::cout << "AllAppsListState Resume" << std::endl;
}

void AllAppsListState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		navbar->HandleEvents(event);

		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
			return;
		}
		else if (event.type == sf::Event::EventType::Resized)
		{
			// TODO: bool resizedWidth, resizedHeight;

			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525u && newSize.y >= 325u)
			{
				sf::FloatRect visibleArea(0.0f, 0.0f, event.size.width, event.size.height);
				*mainView = sf::View(visibleArea);
				app->window->setView(sf::View(visibleArea));

				viewScroller->setSize(sf::Vector2f(event.size.width, event.size.height));
				viewScroller->setCenter(sf::Vector2f(viewScroller->getSize().x / 2, viewScroller->getSize().y / 2));
			}
			else
			{
				if (event.size.width <= 525)
					newSize.x = 525;

				if (event.size.height <= 325)
					newSize.y = 325;

				app->window->setSize(newSize);
			}

			scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, navbar->bar.getSize().y));
			scrollbar.setTrackHeight(app->window->getSize().y - navbar->bar.getSize().y);
			updateScrollThumbSize();

			app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));
		}
		else if (event.type == sf::Event::EventType::KeyPressed && scrollbar.isEnabled)
		{
			if (event.key.code == sf::Keyboard::Key::R && !app->multithreaded_process_running)
			{
				std::cout << "refreshing applist" << std::endl;

				apps.clear();

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) // redownload apps list
				{
					try
					{
						fs::remove(GBL::DIR::appcache + "/index.dat");
					}
					catch (const std::exception& e)
					{
						std::cout << "failed to remove item index:" << std::endl;
						std::cout << e.what() << std::endl;
					}

					Download getNewIndex;
					getNewIndex.setInput("./" + GBL::WEB::APPS + "/index.dat");
					getNewIndex.setOutputDir(GBL::DIR::appcache);
					getNewIndex.setOutputFilename("/index.dat");
					getNewIndex.download();
					getNewIndex.save();
				}

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) // nuke everything and redownload
				{
					try
					{
						fs::remove_all(GBL::DIR::appcache);
						fs::create_directory(GBL::DIR::appcache);

						Download getNewIndex;
						getNewIndex.setInput("./" + GBL::WEB::APPS + "/index.dat");
						getNewIndex.setOutputDir(GBL::DIR::appcache);
						getNewIndex.setOutputFilename("/index.dat");
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
				app->multithread = new std::thread(&AllAppsListState::loadApps, this, std::ref(app->multithreaded_process_finished));

				viewScroller->setCenter(viewScroller->getSize().x / 2, viewScroller->getSize().y / 2);
			}
			else if (event.key.code == sf::Keyboard::Key::Escape)
			{
				if (!app->multithreaded_process_running)
				{
					app->ChangeState(new HomeState);
					return;
				}
				else
				{
					std::cout << "helper is running, not switching states" << std::endl;
				}
			}
		}
		else if (event.type == sf::Event::EventType::MouseButtonReleased && !app->multithreaded_process_running)
		{
			if (event.mouseButton.button == sf::Mouse::Button::Left)
			{
				if (mouseIsOver(navbar->bar))
				{
					for (auto& x : navbar->sections)
						if (mouseIsOver(x))
							if (x.getString() != "all apps" && mouseIsOver(x))
							{
								if (x.getString() == "home")
									app->ChangeState(new HomeState);
								else if (x.getString() == "my apps")
									app->ChangeState(new MyAppListState);
								else if (x.getString() == "settings")
									app->ChangeState(new SettingsState);

								std::cout << "state will be switched" << std::endl;
								return;
							}
				}

				for (size_t i = 0; i < apps.size(); i++)
				{
					if (mouseIsOver(apps[i]->cardShape, viewScroller))
					{
						apps[i]->onClick(event, app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *viewScroller));
					}
				}
			}
		}

		if (scrollbar.isEnabled && !app->multithreaded_process_running)
		{
			if (event.type == sf::Event::EventType::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Button::Left)
				{
					if (mouseIsOver(scrollbar.scrollThumb))
					{
						scrollbar.dragOffset = scrollbar.scrollThumb.getPosition() - sf::Vector2f(sf::Mouse::getPosition(*app->window));
						scrollbar.draggingThumb = true;
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					}
				}
			}
			else if (event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (scrollbar.draggingThumb)
				{
					scrollbar.draggingThumb = false;

					if (mouseIsOver(scrollbar.scrollThumb, mainView))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
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

					if (scrollerBottomPosition < scrollerMaxPosition)
						viewScroller->move(0, scrollbar.scrollJump); // static cast to avoid pixel-imperfect placement of text
					else
						std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << std::endl;

					testScrollBounds();
				}
				else if (event.mouseWheel.delta > 0) // scroll up, or move apps down
				{
					scrollbar.jumpUp();

					if (scrollerTopPosition > scrollerMinPosition)
						viewScroller->move(0, -scrollbar.scrollJump); // static cast to avoid pixel-imperfect placement of text
					else
						std::cout << "cannot scroll view up (" << scrollerTopPosition << " > " << scrollerMaxPosition << ")" << std::endl;

					testScrollBounds();
				}
			}
			else if (event.type == sf::Event::EventType::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Key::Down)
				{
					scrollbar.stepDown();

					if (scrollerBottomPosition < scrollerMaxPosition)
						viewScroller->move(0, scrollbar.scrollStep); // static cast to avoid pixel-imperfect placement of text
					else
						std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << std::endl;

					testScrollBounds();
				}
				else if (event.key.code == sf::Keyboard::Key::Up)
				{
					scrollbar.stepUp();

					if (scrollerTopPosition > scrollerMinPosition)
						viewScroller->move(0, -scrollbar.scrollStep); // static cast to avoid pixel-imperfect placement of text
					else
						std::cout << "cannot scroll view up (" << scrollerTopPosition << " > " << scrollerMaxPosition << ")" << std::endl;

					testScrollBounds();
				}
				else if (event.key.code == sf::Keyboard::Key::Home)
				{
					scrollbar.moveToTop();
					viewScroller->setCenter(viewScroller->getCenter().x, (scrollerMinPosition + viewScroller->getSize().y / 2) - navbar->bar.getSize().y);
				}
				else if (event.key.code == sf::Keyboard::Key::End)
				{
					scrollbar.moveToBottom();
					viewScroller->setCenter(viewScroller->getCenter().x, scrollerMaxPosition - viewScroller->getSize().y / 2);
				}
			}
			else if (event.type == sf::Event::EventType::MouseMoved)
			{
				if (scrollbar.draggingThumb)
				{
					scrollbar.scrollThumb.setPosition(sf::Vector2f(scrollbar.scrollThumb.getPosition().x, sf::Mouse::getPosition(*app->window).y + scrollbar.dragOffset.y));
					if (!scrollbar.boundsCheck()) // scroll from the last legitimate location
						scrollbar.dragOffset = scrollbar.scrollThumb.getPosition() - sf::Vector2f(sf::Mouse::getPosition(*app->window));

					viewScroller->setCenter(sf::Vector2f(viewScroller->getCenter().x, mainView->getCenter().y + ((scrollbar.scrollThumb.getPosition().y - scrollbar.min) * scrollbar.scrollStep)));

					// FIXME: error zone for scrollbar (homestate)
					//				if (sf::Mouse::getPosition(*app->window).x > scrollbar.scrollTrack.getPosition().x + 150 || sf::Mouse::getPosition(*app->window).x < scrollbar.scrollTrack.getPosition().x - 150) // error zone
					//					scrollbar.scrollThumb.setPosition(scrollbar.originalPosition);
				}
				else
				{
					// TODO: find a way to not do this every frame

					if (mouseIsOver(scrollbar.scrollThumb, mainView))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
				}
			}
		}
	}
}

void AllAppsListState::Update()
{
	if (app->multithreaded_process_finished)
	{
		std::cout << "helper thread finished work, joining" << std::endl;
		app->multithread->join();
		app->multithreaded_process_finished = false;
		app->multithreaded_process_running = false;
		delete app->multithread;
	}

	for (size_t i = 0; i < apps.size(); i++)
		apps[i]->update();
}

void AllAppsListState::Draw()
{
	app->window->clear(GBL::theme.palatte.TERTIARY);

	//scrollable
	app->window->setView(*viewScroller);
	for (size_t i = 0; i < apps.size(); i++)
		app->window->draw(*apps[i]);

	//anchored
	app->window->setView(*mainView);

	navbar->Draw();

	if (scrollbar.isEnabled)
		app->window->draw(scrollbar);

	if (app->multithreaded_process_running)
		app->ShowMultiThreadedIndicator();

	app->window->display();
}

const float padding = 10.0f;

void AllAppsListState::loadApps(bool &finishedIndicator)
{
	finishedIndicator = false;

	std::cout << "loading AllApps" << std::endl;

	sf::Clock appLoadTime;

	apps.clear();
	updateScrollThumbSize();
	std::cout << std::endl; // for a line break

	std::string line; // each line of index.dat;

	// TODO: check for index updates
	if (!fs::exists(GBL::DIR::appcache + "index.dat"))
	{
		Download getNewIndex;
		getNewIndex.setInput("./" + GBL::WEB::APPS + "/index.dat");
		getNewIndex.setOutputDir(GBL::DIR::appcache);
		getNewIndex.setOutputFilename("/index.dat");
		getNewIndex.download();
		getNewIndex.save();
	}

	std::ifstream readIndex(GBL::DIR::appcache + "index.dat", std::ios::in);

	SettingsParser getApp;
	getApp.loadFromFile(GBL::DIR::appcache + "index.dat");

	int loopi(0);
	while (std::getline(readIndex, line))
	{
		std::cout << "\n" << "loop#" << loopi << std::endl;

		std::string item_name;
		if (getApp.get(std::to_string(loopi), item_name))
		{
			StoreApp* newItem;

			if (apps.empty())
				newItem = new StoreApp(loopi,
					app->window->getSize().x - (padding - scrollbar.scrollTrack.getSize().x),
					50, // FIXME: magic numbers are bad
					padding, // 10 pixels to the left of the left edge of the screen, minus the size of the scrollTrack
					navbar->bar.getSize().y + padding);
			else
				newItem = new StoreApp(loopi,
					app->window->getSize().x - (padding - scrollbar.scrollTrack.getSize().x),
					50, // FIXME: magic numbers are bad
					padding,
					apps.back()->getPosition().y + apps.back()->getLocalBounds().height + padding);

			apps.push_back(newItem);
			std::cout << std::endl;
		}

		// TODO: apps that have updates available
		// should be moved to the top of the list
		// with a nice lil notification for them
//		if (apps.back()->updateIsAvailable)
//		{
//			StoreApp *updateItem = apps.back; // temporary copy
//			apps.pop_back(); // remove from applist
//			apps.insert(apps.begin(), updateItem); // put in front of applist
//		}

		updateScrollThumbSize();
		loopi += 1;
	}

	readIndex.close();

	std::cout << "finished loading apps" << " (" << apps.size() << " apps loaded in " << appLoadTime.getElapsedTime().asSeconds() << " seconds)" << std::endl;

	app->window->requestFocus();

	finishedIndicator = true;
}

void AllAppsListState::updateScrollThumbSize()
{
	float contentHeight(padding);

	if (!apps.empty())
	{
		if (apps.size() > 1)
			contentHeight = (apps.back()->getPosition().y + apps.back()->getLocalBounds().height) - apps.front()->getPosition().y;
		else
			contentHeight = apps.back()->getLocalBounds().height;

		contentHeight += padding * 2;
	}

	scrollbar.update(contentHeight, viewScroller->getSize().y - navbar->bar.getSize().y);

	for (size_t i = 0; i < apps.size(); i++)
		apps[i]->updateSizeAndPosition(
//			padding * 2 because this makes it the size of the window - 10, 
//			then later it goes moved 10 pixels to the left, negating this
			app->window->getSize().x - (padding * 2) - scrollbar.scrollTrack.getSize().x,
			apps[i]->cardShape.getSize().y,
			padding,
			apps[i]->cardShape.getPosition().y);

	updateScrollLimits();
}

void AllAppsListState::updateScrollLimits()
{
	scrollerTopPosition = viewScroller->getCenter().y - viewScroller->getSize().y / 2 + navbar->bar.getSize().y; // is 40 pixels lower
	scrollerBottomPosition = viewScroller->getCenter().y + viewScroller->getSize().y / 2;
	scrollerMinPosition = navbar->bar.getSize().y; // navbar
	scrollerMaxPosition = scrollbar.contentSize + navbar->bar.getSize().y;
}

void AllAppsListState::testScrollBounds()
{
	updateScrollLimits();

	if (scrollerBottomPosition > scrollerMaxPosition) // clamp viewScroller
	{
		std::cout << "viewScroller went too far down (" << scrollerBottomPosition - scrollerMaxPosition << "), clamping..." << std::endl;
		viewScroller->setCenter(viewScroller->getCenter().x, scrollerMaxPosition - viewScroller->getSize().y / 2);
		updateScrollLimits();
	}

	if (scrollerTopPosition < scrollerMinPosition) // clamp viewScroller
	{
		std::cout << "viewScroller went too far up (" << scrollerMaxPosition - scrollerTopPosition << "), clamping..." << std::endl;
		viewScroller->setCenter(viewScroller->getCenter().x, (scrollerMinPosition + viewScroller->getSize().y / 2) - navbar->bar.getSize().y);
		updateScrollLimits();
	}
}

bool AllAppsListState::mouseIsOver(const sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

bool AllAppsListState::mouseIsOver(const sf::Shape &object, const sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool AllAppsListState::mouseIsOver(const sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

bool AllAppsListState::mouseIsOver(const sf::Text &object, const sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}
