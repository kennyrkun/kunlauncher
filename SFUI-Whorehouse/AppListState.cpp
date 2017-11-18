#include "AppEngine.hpp"
#include "AppState.hpp"
#include "AppListState.hpp"
#include "HomeState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "MessageBox.hpp"
#include "Item.hpp"
#include "Link.hpp"
#include "SettingsParser.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>

AppListState AppListState::AppListState_dontfuckwithme;

namespace fs = std::experimental::filesystem;

void AppListState::Init(AppEngine* app_)
{
	std::cout << "AppListState Init" << std::endl;

	app = app_;

	cardScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	//	scrollbar.create();
	scrollbar.create(app->window);

	tracker1.setSize(sf::Vector2f(5, 5));
	tracker2.setSize(sf::Vector2f(5, 5));
	tracker3.setSize(sf::Vector2f(5, 5));

	tracker1.setOrigin(tracker1.getLocalBounds().width / 2, tracker1.getLocalBounds().height / 2);
	tracker2.setOrigin(tracker2.getLocalBounds().width / 2, tracker2.getLocalBounds().height / 2);
	tracker3.setOrigin(tracker3.getLocalBounds().width / 2, tracker3.getLocalBounds().height / 2);

	tracker1.setFillColor(sf::Color::Green);
	tracker2.setFillColor(sf::Color::Blue);
	tracker3.setFillColor(sf::Color::Red);

	helperThread = new std::thread(&AppListState::loadApps, this);
	helperRunning = true;
	std::cout << "thread launched" << std::endl;
}

void AppListState::Cleanup()
{
	delete cardScroller;

	if (helperRunning)
	{
		std::cout << "waiting on helper thread to finish" << std::endl;
		helperThread->join();
	}

	items.clear();
	links.clear();

	//	delete app; // dont delete app because it's being used by the thing and we need it.
	//	app = nullptr;

	std::cout << "AppListState Cleanup" << std::endl;
}

void AppListState::Pause()
{
	std::cout << "AppListState Pause" << std::endl;
}

void AppListState::Resume()
{
	std::cout << "AppListState Resume" << std::endl;
}

void AppListState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			app->Quit();
		}
		else if (event.type == sf::Event::EventType::Resized && !loadingApps)
		{
			std::cout << "new width: " << event.size.width << std::endl;
			std::cout << "new height: " << event.size.height << std::endl;

			sf::Vector2u newSize(event.size.width, event.size.height);

			if (newSize.x >= 525 && newSize.y >= 325)
			{
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				*mainView = sf::View(visibleArea);
				app->window->setView(sf::View(visibleArea));

				cardScroller->setSize(event.size.width, event.size.height);
				cardScroller->setCenter(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2);

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
		else if (event.type == sf::Event::EventType::MouseWheelMoved && scrollbar.isEnabled && !loadingApps)
		{
			if (event.mouseWheel.delta < 0) // down, or move items up
			{
				scrollbar.moveThumbDown();

				if (scrollerBottomPosition < scrollerMaxPosition)
				{
					cardScroller->move(0, scrollbar.scrollJump);

					updateScrollLimits();

					if (scrollerBottomPosition > scrollerMaxPosition) // clamp cardScroller
					{
						std::cout << "cardScroller went too far down (" << scrollerBottomPosition - scrollerMaxPosition << "), clamping..." << std::endl;
						cardScroller->setCenter(cardScroller->getCenter().x, scrollerMaxPosition - cardScroller->getSize().y / 2 + 8);
						updateScrollLimits();
					}
				}
				else
				{
					std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << std::endl;
				}
			}
			else if (event.mouseWheel.delta > 0) // scroll up, or move items down
			{
				scrollbar.moveThumbUp();

				if (scrollerTopPosition > scrollerMinPosition)
				{
					cardScroller->move(0, -scrollbar.scrollJump);

					updateScrollLimits();

					if (scrollerTopPosition < scrollerMinPosition) // clamp cardScroller
					{
						std::cout << "cardScroller went too far up (" << scrollerMaxPosition - scrollerTopPosition << "), clamping..." << std::endl;
						cardScroller->setCenter(cardScroller->getCenter().x, scrollerMinPosition + cardScroller->getSize().y / 2);
						updateScrollLimits();
					}
				}
				else
				{
					std::cout << "cannot scroll view up (" << scrollerTopPosition << " < " << scrollerMaxPosition << ")" << std::endl;
				}
			}
		}
		else if (event.type == sf::Event::EventType::MouseButtonPressed && !loadingApps)
		{
			if (event.mouseButton.button == sf::Mouse::Button::Left)
			{
				bool clicked(false);

				//links
				for (size_t i = 0; i < items.size(); i++)
				{
					if (items[i]->downloaded)
					{
						if (mouseIsOver(items[i]->removeButton))
						{
							MessageBox::Options modOptions;
							modOptions.title = "Confirm Deletion";
							modOptions.text = "Delete \"" + items[i]->name.getString() + "\"?";
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

								threads.push_back(std::thread(&Item::deleteFiles, items[i]));

								clicked = true;
							}
						}
						else if (mouseIsOver(items[i]->redownloadButton))
						{
							std::cout << "redownload button pressed" << std::endl;

							threads.push_back(std::thread(&Item::updateItem, items[i]));

							clicked = true;
						}
						else if (mouseIsOver(items[i]->launchButton))
						{
							std::cout << "launch button pressed" << std::endl;

							items[i]->openItem();

							clicked = true;
						}
					}
					else
					{
						if (mouseIsOver(items[i]->downloadButton))
						{
							std::cout << "download button pressed" << std::endl;

							threads.push_back(std::thread(&Item::download, items[i]));

							clicked = true;
						}
					}

					if (clicked)
						break;
				}

				//links
				for (size_t i = 0; i < links.size(); i++)
				{
					if (mouseIsOver(links[i]->linkText) || mouseIsOver(links[i]->followLinkButton))
					{
						// follow link
						links[i]->follow();

						break;
					}
				}
			}
			else if (event.key.code == sf::Mouse::Button::Right)
			{
				app->ChangeState(HomeState::Instance());
			}
		}
		else if (event.type == sf::Event::EventType::MouseButtonReleased && !loadingApps)
		{
			if (scrollbar.thumbDragging)
			{
				scrollbar.thumbDragging = false;
				scrollbar.scrollThumb.setFillColor(GBL::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER);
			}
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::R && !loadingApps)
			{
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl))
				{
					try
					{
						fs::remove(".\\" + GBL::DIR::BASE + GBL::DIR::APPS);
					}
					catch (const std::exception& e)
					{
						std::cout << "failed to remove apps folder: " << std::endl;
						std::cout << e.what() << std::endl;
					}
				}

				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) // redownload items list
				{
					try
					{
						fs::remove(".\\" + GBL::DIR::BASE + GBL::DIR::APPS + "\\index.dat");
					}
					catch (const std::exception& e)
					{
						std::cout << "failed to remove item index:" << std::endl;
						std::cout << e.what() << std::endl;
					}

					Download2 getNewIndex;
					getNewIndex.setInput(".\\" + GBL::WEB::BASE + "\\index.dat");
					getNewIndex.setOutputDir(".\\" + GBL::DIR::BASE + GBL::DIR::APPS);
					getNewIndex.setOutputFilename("index.dat");
					getNewIndex.download();
					getNewIndex.save();
				}

				std::cout << "refreshing applist" << std::endl;

				links.clear();
				items.clear();

				helperThread = new std::thread(&AppListState::loadApps, this);
				helperDone = false;
				helperRunning = true;

				cardScroller->setCenter(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2);
			}
			else if (event.key.code == sf::Keyboard::Key::Escape)
			{
				if (helperRunning)
				{
					app->ChangeState(HomeState::Instance());
				}
				else
				{
					std::cout << "helper is running, not switching states" << std::endl;
				}
			}
		}
	}
}

void AppListState::Update()
{
	if (helperDone && !helperRunning)
	{
		std::cout << "helper done, joining" << std::endl;
		helperThread->join();

		helperDone = false;
		helperRunning = false;
	}

	for (size_t i = 0; i < threads.size(); i++)
	{
		if (threads[i].joinable())
		{
			std::cout << "joining" << std::endl;

			threads[i].detach();
			threads.erase(threads.begin() + i);
		}
	}
}

void AppListState::Draw()
{
	app->window->clear(GBL::COLOR::BACKGROUND);

	//scrollable
	app->window->setView(*cardScroller);
	for (size_t i = 0; i < items.size(); i++)
		items[i]->draw();

	for (size_t i = 0; i < links.size(); i++)
		links[i]->draw();

	//anchored
	app->window->setView(*mainView);

	if (scrollbar.isEnabled)
		app->window->draw(scrollbar);

	app->window->display();
}

void AppListState::loadApps() // TOOD: this.
{
	loadingApps = true;

	items.clear();
	links.clear();
	updateScrollThumbSize();
	std::cout << std::endl; // for a line break

	bool comesAfterLink(false), comesAfterItem(false);
	std::string line; // each line of index.dat;

	std::ifstream readIndex(".\\" + GBL::DIR::BASE + GBL::DIR::APPS + "index.dat", std::ios::in);

	int loopi(0);
	while (std::getline(readIndex, line))
	{
		std::cout << loopi << " - " << line << std::endl;

		// syntax should look like this: APP:"appname"
		// or for links, LINK:"text"TO"link.com"
		if (line[0] == 'A' && line[1] == 'P' && line[2] == 'P') // it's an app
		{
			line.erase(0, 3); // remove APP
			line.erase(0, 1); // remove :
			line.erase(0, 1); // remove first "
			line.erase(line.length() - 1, line.length()); // remove last "

			if (comesAfterLink)
			{
				std::cout << "(item after link)" << std::endl;

				Item* newItem = new Item(line, app->window,
					(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16),
					app->window->getSize().y,
					(app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2),
					items.back()->cardShape.getPosition().y + items.back()->totalHeight /* PADDING */);

				items.push_back(newItem);
				std::cout << std::endl;
			}
			else // not after a link
			{
				std::cout << "(item not after link)" << std::endl;

				Item* newItem;

				if (items.empty())
					newItem = new Item(line, app->window,
					(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16), 
					app->window->getSize().y, // I'm not sure what this is for?????
					(app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), // mid-window, excluding scrollbar size
					(75 / 2) + 10);
				else
					newItem = new Item(line, app->window,
					(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16),
					app->window->getSize().y, // I'm not sure what this is for?????
					(app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), // the middle of the window (exluding the size of the scrollbar)
					items.back()->cardShape.getPosition().y + items.back()->totalHeight + 10 /* PADDING */);

				items.push_back(newItem);
				std::cout << std::endl;
			}

			comesAfterItem = true;
			comesAfterLink = false;
		}
		else if (line[0] == 'L' && line[1] == 'I' && line[2] == 'N' && line[3] == 'K')
		{
			std::cout << "it's a link" << std::endl;

			std::string linkText, linkRel, linkFull(line);

			line.erase(0, 4); // remove LINK
			line.erase(0, 1); // remove :
			line.erase(0, 1); // remove first "
			line.erase(line.find('"'), line.length()); // remove everything after last " in link text
			linkText = line;

			line = linkFull;
			line.erase(0, 4); // remove LINK
			line.erase(0, 1); // remove :
			line.erase(0, 1); // remove first " in like name
			line.erase(0, line.find('"') + 1); // remove last " in link name and everything before
			line.erase(0, 2); // remove TO
			line.erase(0, 1); // remove first "
			line.erase(line.length() - 1, line.length()); // remove last "
			linkRel = line;

			if (comesAfterItem)
			{
				std::cout << "(link after item)" << std::endl;

				Link* newLink = new Link(linkText, linkRel, app->window, items.back()->cardShape.getPosition().y + 66); // we don't check to make sure this isn't empty, because we know there's an item before it.
				links.push_back(newLink);
			}
			else // after a link or first of  either
			{
				Link* newLink;

				if (links.empty())
				{
					newLink = new Link(linkText, linkRel, app->window, 28);

					std::cout << "(link not after item, first link)" << std::endl;
				}
				else // not the first link
				{
					newLink = new Link(linkText, linkRel, app->window, links.back()->cardShape.getPosition().y + 48);

					std::cout << "(link after link, not after item)" << std::endl;
				}

				links.push_back(newLink);
				std::cout << std::endl;
			}

			comesAfterItem = false;
			comesAfterLink = true;
		}
		else
		{
			std::cout << "String is malformed! Skipping! (" << line << ")\n" << std::endl;
			loopi += 1;
			continue;
		}

		updateScrollThumbSize();
		loopi += 1;
	}

	std::cout << "fiinished loading apps" << " (" << items.size() << " items, " << links.size() << " links loaded)" << std::endl;
	helperDone = true;

	loadingApps = false;
}

void AppListState::updateScrollThumbSize()
{
	// set the scrollbar size

	float contentHeight(0);
	for (size_t i = 0; i < items.size(); i++)
		contentHeight += items[i]->totalHeight + 10;

	for (size_t i = 0; i < links.size(); i++)
		contentHeight += links[i]->totalHeight + 10;

	scrollbar.update(contentHeight, cardScroller->getSize().y);

	for (size_t i = 0; i < items.size(); i++)
		items[i]->updateSize(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16, app->window->getSize().y, (app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), items[i]->cardShape.getPosition().y + 43);

	for (size_t i = 0; i < links.size(); i++)
		links[i]->updateSize(app->window->getSize().x - scrollbar.scrollbar.getSize().x - 16, app->window->getSize().y, (app->window->getSize().x / 2) - (scrollbar.scrollbar.getSize().x / 2), links[i]->cardShape.getPosition().y + links[i]->cardShape.getLocalBounds().height + 8);

	updateScrollLimits();
}

void AppListState::updateScrollLimits()
{
	scrollerTopPosition = cardScroller->getCenter().y - cardScroller->getSize().y / 2;
	scrollerBottomPosition = cardScroller->getCenter().y + cardScroller->getSize().y / 2;
	scrollerMinPosition = 0;
	scrollerMaxPosition = scrollbar.contentHeight;
}

bool AppListState::mouseIsOver(const sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}

bool AppListState::mouseIsOver(const sf::Shape &object, const sf::View* view)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *view)))
		return true;
	else
		return false;
}

bool AppListState::mouseIsOver(const sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}
