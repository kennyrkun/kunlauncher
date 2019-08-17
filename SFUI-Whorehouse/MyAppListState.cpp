#include "AppEngine.hpp"
#include "HomeState.hpp"
#include "MyAppListState.hpp"
#include "AllAppsListState.hpp"
#include "SettingsState.hpp"

#include "Globals.hpp"
#include "MessageBox.hpp"
#include "MyApp.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <experimental/filesystem>
#include <cctype> // isdigit, toupper, tolower

namespace fs = std::experimental::filesystem;

void MyAppListState::Init(AppEngine* app_)
{
	std::cout << "MyAppListState Init" << std::endl;
	app = app_;

	app->navbar->select("my apps");

	viewScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());

	scrollbar.create(app->window);
	scrollbar.setTrackHeight(app->window->getSize().y - app->navbar->bar.getLocalBounds().height);
	scrollbar.setPosition(sf::Vector2f(scrollbar.getPosition().x, app->navbar->bar.getSize().y));

	app->multithreaded_process_finished = false;
	app->multithreaded_process_running = true;
	app->multithread = new std::thread(&MyAppListState::loadApps, this, std::ref(app->multithreaded_process_finished));

	app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));
}

void MyAppListState::Cleanup()
{
	std::cout << "Cleaning up MyAppListState." << std::endl;

	app->am.clearTasks();

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

	delete viewScroller;
	delete mainView;

	std::cout << "MyAppListState cleaned up." << std::endl;
}

void MyAppListState::Pause()
{
	std::cout << "MyAppListState Pause" << std::endl;
}

void MyAppListState::Resume()
{
	app->navbar->select("my apps");
	std::cout << "MyAppListState Resume" << std::endl;
}

void MyAppListState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		app->navbar->HandleEvents(event);

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

			scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, app->navbar->bar.getSize().y));
			scrollbar.setTrackHeight(app->window->getSize().y - app->navbar->bar.getSize().y);
			updateScrollThumbSize();

			app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));
		}
		else if (event.type == sf::Event::EventType::MouseButtonReleased && !app->multithreaded_process_running)
		{
			if (event.mouseButton.button == sf::Mouse::Button::Left)
			{
				if (mouseIsOver(app->navbar->bar))
				{
					for (const auto [name, text] : app->navbar->sections)
						if (mouseIsOver(text))
							if (name != "my apps" && mouseIsOver(text))
							{
								if (name == "home")
									app->ChangeState(new HomeState);
								else if (name == "all apps")
									app->ChangeState(new AllAppsListState);
								else if (name == "settings")
									app->ChangeState(new SettingsState);

								std::cout << "state will be switched" << std::endl;
								return;
							}
				}

				for (size_t i = 0; i < apps.size(); i++)
				{
					if (mouseIsOver(apps[i]->cardShape, viewScroller))
					{
						int id = apps[i]->onClick(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *viewScroller));

						if (id == MyApp::CLICKBACK::DeleteFiles)
						{
							AsyncTask* tt = new AsyncTask;
							tt->future = std::async(std::launch::async, &MyAppListState::removeAppFromList, this, apps[i]);
							GBL::threadManager.addTask(tt);
						}
						else if (id == MyApp::CLICKBACK::OpenFiles)
						{
							AsyncTask* tt = new AsyncTask;
							tt->future = std::async(std::launch::async, &MyApp::openItem, apps[i]);
							GBL::threadManager.addTask(tt);
						}
						else if (id == MyApp::CLICKBACK::RedownloadFiles)
						{
							AsyncTask* tt = new AsyncTask;
							tt->future = std::async(std::launch::async, &MyApp::redownload, apps[i]);
							GBL::threadManager.addTask(tt);
						}
					}
				}
			}
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::R && !app->multithreaded_process_running) // RELOAD APPS LIST
			{
				std::cout << "refreshing myappslist" << std::endl;

				app->am.clearTasks();

				for (size_t i = 0; i < apps.size(); i++)
					delete apps[i];
				apps.clear();

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) // nuke everything and redownload
				{
					try
					{
						fs::remove_all(GBL::DIR::apps);
						fs::create_directory(GBL::DIR::apps);
					}
					catch (const std::exception& e)
					{
						std::cerr << "failed to remove apps folder: " << std::endl;
						std::cerr << e.what() << std::endl;
					}
				}

				app->multithreaded_process_finished = false;
				app->multithreaded_process_running = true;
				app->multithread = new std::thread(&MyAppListState::loadApps, this, std::ref(app->multithreaded_process_finished));

				viewScroller->setCenter(viewScroller->getSize().x / 2, viewScroller->getSize().y / 2);
				scrollbar.moveToTop();
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
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOLD);
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
					viewScroller->setCenter(viewScroller->getCenter().x, (scrollerMinPosition + viewScroller->getSize().y / 2) - app->navbar->bar.getSize().y);
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

	// if an app has been deleted, remove it from the list.
	// this is here because we can remove apps from outside of the state
	for (size_t i = 0; i < apps.size(); i++)
	{
		// TODO: move this into a sort of event queue
		if (!apps[i]->info.downloaded)
		{
			app->multithreaded_process_finished = false;
			app->multithreaded_process_running = true;
			app->multithread = new std::thread(&MyAppListState::removeAppFromList, this, apps[i]);
			i--;
		}
		else // call update like normal
			apps[i]->update();
	}
}

void MyAppListState::Draw()
{
	app->window->clear(GBL::theme.palatte.TERTIARY);

	//scrollable
	app->window->setView(*viewScroller);
	for (size_t i = 0; i < apps.size(); i++)
		app->window->draw(*apps[i]);

	for (auto& x : apps)
		app->window->draw(*x);

	//anchored
	app->window->setView(*mainView);

	app->navbar->Draw();

	if (scrollbar.isEnabled)
		app->window->draw(scrollbar);

	if (app->multithreaded_process_running)
		app->ShowMultiThreadedIndicator();

	app->window->display();
}

const float padding = 10.f;

void MyAppListState::loadApps(bool &finishedIndicator)
{
	finishedIndicator = false;

	sf::Clock appLoadTime;

	std::cout << "loading MyApps" << std::endl;
	std::cout << std::endl; // for a line break

	apps.clear();
	updateScrollThumbSize();

	std::vector<std::string> appList = getDirectories(GBL::DIR::apps);

	// check for invalid appids
	for (size_t i = 0; i < appList.size(); i++)
		// go through each character of the current string
		for (size_t j = 0; j < appList[i].size(); j++)
			// if the character isn't a digit
			if (!std::isdigit(appList[i][j]))
			{
				// remove the string from the applist
				// as it cannot be a real app, since
				// they are identified using only numbers
				appList.erase(appList.begin() + i);
				i--; // move back one (because we shortened the vector)
				// TODO: make sure this doesn't cause any bugs
				break; // move on to the next app
			}

	std::cout << std::endl; // for a line break

	sf::Vector2f nextPosition = { padding, app->navbar->bar.getSize().y + padding };

	for (size_t i = 0; i < appList.size(); i++)
	{
		MyApp* newItem;
		int appid = std::stoi(appList[i]);

		if (apps.empty())
			newItem = new MyApp(appid,
				app->window->getSize().x - (padding - scrollbar.scrollTrack.getSize().x),
				75,
				nextPosition);
		else
		{
			newItem = new MyApp(appid,
				app->window->getSize().x - (padding - scrollbar.scrollTrack.getSize().x),
				75,
				nextPosition);

			nextPosition.y = ((75 + padding) * i) + app->navbar->bar.getSize().y + padding;
		}

		newItem->setPosition(sf::Vector2f(app->window->getSize().x + padding, nextPosition.y));

		apps.push_back(newItem);

		// TODO: don't update apps if they're not on screen
		app->am.addAppTranslationTask(newItem, nextPosition, EaseType::CubicEaseOut, 1000);

		std::cout << std::endl;

		// TODO: apps that have updates available
		// should be moved to the top of the list
		// with a nice lil notification for them
//		if (apps.back()->updateIsAvailable)
//		{
//			App *updateItem = apps.back; // temporary copy
//			apps.pop_back(); // remove from applist
//			apps.insert(apps.begin(), updateItem); // put in front of applist
//		}

		updateScrollThumbSize();
	}

	std::cout << "finished loading apps" << " (" << apps.size() << " apps in " << appLoadTime.getElapsedTime().asSeconds() << " seconds)" << std::endl;

	app->window->requestFocus();

	finishedIndicator = true;
}

void MyAppListState::updateScrollThumbSize()
{
	// set the scrollTrack size

	float contentHeight(10); // last padding on bottom
	for (size_t i = 0; i < apps.size(); i++)
		contentHeight += apps[i]->getLocalBounds().height + 10;

	scrollbar.update(contentHeight, viewScroller->getSize().y);

	for (size_t i = 0; i < apps.size(); i++)
		apps[i]->updateSizeAndPosition(app->window->getSize().x - (padding * 2) - scrollbar.scrollTrack.getSize().x,
		75, 
		apps[i]->getPosition().x, 
		apps[i]->getPosition().y);

	updateScrollLimits();
}

void MyAppListState::updateScrollLimits()
{
	scrollerTopPosition = viewScroller->getCenter().y - viewScroller->getSize().y / 2 + app->navbar->bar.getSize().y; // is 40 pixels lower
	scrollerBottomPosition = viewScroller->getCenter().y + viewScroller->getSize().y / 2;
	scrollerMinPosition = app->navbar->bar.getSize().y; // navbar
	scrollerMaxPosition = scrollbar.contentSize + app->navbar->bar.getSize().y;
}

void MyAppListState::testScrollBounds()
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
		viewScroller->setCenter(viewScroller->getCenter().x, (scrollerMinPosition + viewScroller->getSize().y / 2) - app->navbar->bar.getSize().y);
		updateScrollLimits();
	}
}

void MyAppListState::removeAppFromList(MyApp* whatApp)
{
	std::vector<MyApp*>::iterator it = std::find(apps.begin(), apps.end(), whatApp);
	int index = std::distance(apps.begin(), it);

	/*
	sf::Vector2f newpos = { app->window->getSize().x + padding, whatApp->getPosition().y };
	int id = app->am.addAppTranslationTask(whatApp, newpos, EaseType::CubicEaseOut, 500);
	sf::sleep(sf::seconds(1.2)); // wait to delete until the thing is actually gone
	*/

	apps.erase(it);

	whatApp->deleteFiles();
	delete whatApp;

	for (size_t i = index; i < apps.size(); i++)
		app->am.addAppTranslationTask(apps[i], sf::Vector2f(padding, ((75 + padding) * i) + app->navbar->bar.getSize().y + padding), EaseType::CubicEaseOut, 600);

	updateScrollThumbSize();
	viewScroller->setCenter(mainView->getCenter());

	/*
	scrollDifference = (75 + 10) * scrollStep;

	int pos = scrollbar.scrollThumb.getPosition().y;
	viewScroller->setCenter(sf::Vector2f(app->window->getSize().x / 2, app->window->getSize().x / 2));
	viewScroller->move(0, pos * scrollbar.scrollJump);
	*/
}

bool MyAppListState::mouseIsOver(const sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	
	return false;
}

bool MyAppListState::mouseIsOver(const sf::Shape &object, const sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;

	return false;
}

bool MyAppListState::mouseIsOver(const sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;

	return false;
}

bool MyAppListState::mouseIsOver(const sf::Text &object, const sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;

	return false;
}

std::vector<std::string> MyAppListState::getDirectories(const std::string& s)
{
	// we don't use the app names in their folders anymore, instead the numerical appids
	// since this is the case, should this function be return a vector of integers?
	// string is probably the most ideal, in the case that a folder has a the wrong name.

	std::vector<std::string> r;
	for (auto& p : fs::directory_iterator(s))
		if (p.status().type() == fs::file_type::directory)
			r.push_back( p.path().string().substr( GBL::DIR::apps.size(), p.path().string().size() ) );
	return r;
}
