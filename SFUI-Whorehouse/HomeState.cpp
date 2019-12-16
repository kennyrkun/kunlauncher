#include "AppEngine.hpp"
#include "HomeState.hpp"
#include "AllAppsListState.hpp"
#include "MyAppListState.hpp"
#include "SettingsState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "App.hpp"

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

// FIXME: crash after going all the way to the end of the news list and then closing the app

// TODO: back and forward buttons for news

namespace fs = std::experimental::filesystem;

void HomeState::Init(AppEngine* app_)
{
	std::cout << "HomeState Init" << std::endl;
	app = app_;

	app->navbar->select("home");

	viewScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());

	scrollbar.create(app->window);
	scrollbar.setTrackHeight(app->window->getSize().y - app->navbar->bar.getLocalBounds().height);
	scrollbar.setPosition(sf::Vector2f(scrollbar.getPosition().x, app->navbar->bar.getSize().y));
	scrollbar.scrollTrack.setFillColor(GBL::theme.palatte.SCROLLBAR);
	scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);

	homeStatus.setFont(SFUI::Theme::getFont());
	homeStatus.setCharacterSize(16);
	homeStatus.setOrigin(sf::Vector2f( static_cast<int>(homeStatus.getLocalBounds().width / 2.0f), static_cast<int>(homeStatus.getLocalBounds().height / 2.0f) ));
	homeStatus.setPosition(mainView->getCenter());

	// TODO: define this style in Theme
	sf::Color linkColour = sf::Color(0, 170, 232);

	previousText.setFont(SFUI::Theme::getFont());
	previousText.setCharacterSize(16);
	previousText.setStyle(sf::Text::Style::Underlined);
	previousText.setFillColor(linkColour);
	previousText.setString("previous");

	nextText.setFont(SFUI::Theme::getFont());
	nextText.setCharacterSize(16);
	nextText.setStyle(sf::Text::Style::Underlined);
	nextText.setFillColor(linkColour);
	nextText.setString("next");

	enableNews.setFont(SFUI::Theme::getFont());
	enableNews.setCharacterSize(16);
	enableNews.setStyle(sf::Text::Style::Underlined);
	enableNews.setFillColor(linkColour);

	app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));

	if (app->settings.news.enabled)
	{
		setStatusText("Loading News...", sf::Vector2f(app->window->getSize().x / 2, app->navbar->bar.getPosition().y + 60));

		app->multithreaded_process_running = true;
		app->multithreaded_process_finished = false;
		app->multithread = new std::thread(&HomeState::loadNews, this, std::ref(app->multithreaded_process_finished), newsLoadedDist.first, newsLoadedDist.second);
	}
	else
	{
		homeStatus.setCharacterSize(26);
		setStatusText("News disabled.");

		enableNews.setString("enable once");
		enableNews.setOrigin(enableNews.getLocalBounds().width / 2, enableNews.getLocalBounds().height / 2);
		enableNews.setPosition(sf::Vector2f(mainView->getCenter().x, static_cast<int>(mainView->getCenter().y + 30)));
	}

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

	for (size_t i = 0; i < newses.size(); i++)
		delete newses[i];
	newses.clear();

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
	app->navbar->select("home");
	std::cout << "HomeState resumed" << std::endl;
}

void HomeState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		app->navbar->HandleEvents(event);

		for (size_t i = 0; i < newses.size(); i++)
			newses[i]->HandleEvents(event);

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
				sf::FloatRect visibleArea(0.0f, 0.0f, event.size.width, event.size.height);
				*mainView = sf::View(visibleArea);
				app->window->setView(sf::View(visibleArea));

				viewScroller->setSize(sf::Vector2f(event.size.width, event.size.height));
				viewScroller->setCenter(sf::Vector2f(viewScroller->getSize().x / 2, viewScroller->getSize().y / 2));

				bool scroll = scrollbar.isEnabled;

				scrollbar.setTrackHeight(app->window->getSize().y - app->navbar->bar.getLocalBounds().height);
				scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, app->navbar->bar.getLocalBounds().height));
				updateScrollThumbSize();

				if (scroll != scrollbar.isEnabled)
				{
					// TODO: update wrapSfText for texts
				}

				app->SetMultiThreadedIndicatorPosition(sf::Vector2f(20, app->window->getSize().y - 20));
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
		else if (event.type == sf::Event::EventType::MouseButtonReleased && !app->multithreaded_process_running)
		{
			if (event.mouseButton.button == sf::Mouse::Button::Left)
			{
				if (mouseIsOver(app->navbar->bar))
				{
					for (const auto [name, text] : app->navbar->sections)
						if (mouseIsOver(text))
							if (name != "home" && mouseIsOver(text))
							{
								if (name == "my apps")
									app->ChangeState(new MyAppListState);
								else if (name == "all apps")
									app->ChangeState(new AllAppsListState);
								else if (name == "settings")
									app->ChangeState(new SettingsState);

								std::cout << "state will be switched" << std::endl;
								return;
							}
				}

				if (!app->multithreaded_process_running)
				{
					if (mouseIsOver(enableNews, viewScroller))
					{
						// the news is not enabled, and the newsInteraction probably says "enable once"
						if (!app->settings.news.enabled)
						{
							app->settings.news.enabled= true;

							// TODO: save this setting

							setStatusText("Loading News...", sf::Vector2f(app->window->getSize().x / 2, app->navbar->bar.getPosition().y + 60));

							enableNews.setPosition(sf::Vector2f(-999, -999));

							app->multithreaded_process_running = true;
							app->multithreaded_process_finished = false;
							app->multithread = new std::thread(&HomeState::loadNews, this, std::ref(app->multithreaded_process_finished), 0, 2);
						}
					}
					else if (mouseIsOver(nextText, viewScroller))
					{
						setStatusText("Loading News...", sf::Vector2f(app->window->getSize().x / 2, app->navbar->bar.getPosition().y + 60));

						app->multithreaded_process_running = true;
						app->multithreaded_process_finished = false;
						app->multithread = new std::thread(&HomeState::loadNews, this, std::ref(app->multithreaded_process_finished), newsLoadedDist.first += 2, newsLoadedDist.second += 2);

						viewScroller->setCenter(app->window->getView().getCenter());
						scrollbar.scrollThumb.setPosition(sf::Vector2f(scrollbar.scrollThumb.getPosition().x, scrollbar.scrollbarTopPosition - (scrollbar.scrollThumb.getPosition().y / 2)));
					}
					else if (mouseIsOver(previousText, viewScroller))
					{
						setStatusText("Loading News...", sf::Vector2f(app->window->getSize().x / 2, app->navbar->bar.getPosition().y + 60));

						app->multithreaded_process_running = true;
						app->multithreaded_process_finished = false;
						app->multithread = new std::thread(&HomeState::loadNews, this, std::ref(app->multithreaded_process_finished), newsLoadedDist.first -= 2, newsLoadedDist.second -= 2);

						viewScroller->setCenter(app->window->getView().getCenter());
						scrollbar.scrollThumb.setPosition(sf::Vector2f(scrollbar.scrollThumb.getPosition().x, scrollbar.scrollbarTopPosition - (scrollbar.scrollThumb.getPosition().y / 2)));
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
//					if (sf::Mouse::getPosition(*app->window).x > scrollbar.scrollTrack.getPosition().x + 150 || sf::Mouse::getPosition(*app->window).x < scrollbar.scrollTrack.getPosition().x - 150) // error zone
//						scrollbar.scrollThumb.setPosition(scrollbar.originalPosition);
				}
				else
				{
					// TODO: find a way to not change scrollbar color every frame

					if (mouseIsOver(scrollbar.scrollThumb, mainView))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
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

	app->navbar->Update();

	for (size_t i = 0; i < newses.size(); i++)
		newses[i]->Update();
}

void HomeState::Draw()
{
	app->window->clear(GBL::theme.palatte.TERTIARY);

	app->window->setView(*viewScroller);

	for (size_t i = 0; i < newses.size(); i++)
		app->window->draw(*newses[i]);

	app->window->draw(previousText);
	// FIXME: crash here
	app->window->draw(nextText);
	app->window->draw(enableNews);

	// non-moving items
	app->window->setView(*mainView);

	app->navbar->Draw();
	app->window->draw(scrollbar);

	// FIXME: fix this little piece of shit
	app->window->draw(homeStatus);

	if (app->multithreaded_process_running)
		app->ShowMultiThreadedIndicator();

	app->window->display();
}

const float padding = 25;

void HomeState::loadNews(bool &finishedIndicator, int loadFrom, int loadTo)
{
	finishedIndicator = false;
	std::cout << "loading home news" << std::endl << std::endl;

	previousText.setString("");
	nextText.setString("");

	// clear the newses first
	for (size_t i = 0; i < newses.size(); i++)
		delete newses[i];
	newses.clear();

	homeStatus.setCharacterSize(16);
	setStatusText("Downloading news...");

	Download getNews;
	getNews.setInput("./" + GBL::WEB::NEWS + "/news.txt");
	getNews.setOutputDir(GBL::DIR::installDir);
	getNews.setOutputFilename("/news.txt");

	if (getNews.download() == Download::Status::Success)
	{
		getNews.save();
	}
	else
	{
		std::cerr << "failed to download news" << std::endl;
		setStatusText("Failed to download news.");

		finishedIndicator = true;
		return;
	}

	setStatusText("Parsing News...");

	std::ifstream readIndex(GBL::DIR::installDir + "news.txt", std::ios::in);
	std::string line; // each line of index.dat;
	int loopi(0);
	while (std::getline(readIndex, line))
	{
		std::cout << loopi << " - " << line << std::endl;

		if (line == "-----------------------------")
		{
			std::cout << "end of current news" << std::endl;

			loopi += 1;
			continue;
		}

		std::string title = line, text;

		// parse the piece of news
		while (true)
		{
			std::string newline;
			std::getline(readIndex, newline);

			if (newline == "-----------------------------")
			{
				text.erase(text.length() - 1, text.length());

				std::cout << "=============news=============" << std::endl;
				std::cout << title << std::endl;
				std::cout << text << std::endl;

				break;
			}
			else
			{
				if (newline.empty())
					text.append("\n");
				else
					text.append(newline + "\n");
			}
		}

		// don't load anything if it's before the loadFrom mark
		if (loopi >= loadFrom)
		{
			News* newsNews = new News(title, text, app->window);

			if (newses.empty())
				newsNews->setPosition(sf::Vector2f(10, 50));
			else
				newsNews->setPosition(sf::Vector2f(10, newses.back()->getPosition().y + newses.back()->getLocalHeight() + padding));

			newses.push_back(newsNews);

			updateScrollThumbSize();
		}
		
		loopi += 1;
		if (loopi == loadTo)
			break; // only load this many (default 10)
	}
	readIndex.close();

	nextText.setString("next");

	if (loadFrom != 0)
		previousText.setString("previous");

	previousText.setPosition(newses.back()->getPosition().x, newses.back()->getPosition().y + newses.back()->getLocalHeight() + 10);
	nextText.setPosition(newses.back()->getPosition().x + previousText.getGlobalBounds().width + 10, newses.back()->getPosition().y + newses.back()->getLocalHeight() + 10);


	std::cout << "loaded " << newses.size() << " newses" << std::endl;

	setStatusText("");
	finishedIndicator = true;
}

void HomeState::updateScrollThumbSize()
{
	float contentHeight(0); // 20

	if (!newses.empty())
	{
		for (size_t i = 0; i < newses.size(); i++)
		{
			contentHeight += newses[i]->getLocalHeight() + padding;
		}

		contentHeight += 30; // the last one
	}

	// the second parameter of this should be something like `viewScroller->getSize().y
	// however, since the viewport is 40 pixels shorter, we will add a - 40
	scrollbar.update(contentHeight, viewScroller->getSize().y - app->navbar->bar.getSize().y);

	// TODO: update scroll limits separately
	updateScrollLimits();
}

void HomeState::updateScrollLimits()
{
	scrollerTopPosition = viewScroller->getCenter().y - viewScroller->getSize().y / 2 + app->navbar->bar.getSize().y; // is 40 pixels lower
	scrollerBottomPosition = viewScroller->getCenter().y + viewScroller->getSize().y / 2;
	scrollerMinPosition = app->navbar->bar.getSize().y; // navbar
	scrollerMaxPosition = scrollbar.contentSize + app->navbar->bar.getSize().y;
}

void HomeState::testScrollBounds()
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

bool HomeState::mouseIsOver(sf::Text &object, sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool HomeState::mouseIsOver(const sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}

void HomeState::setStatusText(const std::string& string)
{
	homeStatus.setString(string);
	// FIXME: there is a crash here
	homeStatus.setOrigin(sf::Vector2f(static_cast<int>(homeStatus.getLocalBounds().width / 2.0f), static_cast<int>(homeStatus.getLocalBounds().height / 2.0f)));
}

void HomeState::setStatusText(const std::string& string, const sf::Vector2f& pos)
{
	homeStatus.setString(string);
	homeStatus.setOrigin(sf::Vector2f(static_cast<int>(homeStatus.getLocalBounds().width / 2.0f), static_cast<int>(homeStatus.getLocalBounds().height / 2.0f)));
	homeStatus.setPosition(pos);
}
