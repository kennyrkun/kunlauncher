#include "AppEngine.hpp"
#include "AppState.hpp"
#include "AppListState.hpp"
#include "HomeState.hpp"

#include "Download.hpp"
#include "Modal.hpp"
#include "Item.hpp"
#include "Link.hpp"
#include "constants.hpp"

#include <SFML\Graphics.hpp>
#include <SFML\Network.hpp>
#include <iostream>
#include <fstream>
#include <experimental\filesystem>

AppListState AppListState::AppListState_dontfuckwithme;

std::vector<std::thread> threads;

void AppListState::Init(AppEngine* app_)
{
	std::cout << "AppListState Init" << std::endl;

	app = app_;

	bool isReady(false);

	app->window->setTitle("KunLauncher " + CONST::VERSION);

	cardScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());

	scrollbar.create(app->window);
	
	helperThread = new std::thread(&AppListState::loadApps, this);
	helperRunning = true;
	std::cout << "thread launched" << std::endl;
}

void AppListState::Cleanup()
{
	delete cardScroller;
//	delete app; // dont delete app because it's being used by the thing and we need it.
//	app = nullptr;

	std::cout << "AppListState Cleanup" << std::endl;
}

void AppListState::Pause()
{
	printf("AppListState Pause\n");
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

				cardScroller->setSize(event.size.width, event.size.height);
				cardScroller->setCenter(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2);

				// set the scrollbar size
				updateScrollThumb();

				for (size_t i = 0; i < items.size(); i++)
					items[i]->update(items[i]->cardShape.getPosition().y);

				for (size_t i = 0; i < links.size(); i++)
					links[i]->update();
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
		else if (event.type == sf::Event::EventType::MouseMoved)
		{
			if (!scrollbar.thumbDragging)
			{
				if (mouseIsOver(scrollbar.scrollThumb))
				{
					scrollbar.scrollThumb.setFillColor(sf::Color(158, 158, 158));
				}
				else
				{
					scrollbar.scrollThumb.setFillColor(sf::Color(110, 110, 110));
				}
			}
		}
		else if (event.type == sf::Event::EventType::MouseWheelMoved) // thanks sfconsole
		{
//			std::cout << "center x: " << app->window->getView().getCenter().x << std::endl;
//			std::cout << "center y: " << app->window->getView().getCenter().y << std::endl;
//			std::cout << "size x: " << app->window->getView().getSize().x << std::endl;
//			std::cout << "size y: " << app->window->getView().getSize().y << std::endl;

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
				bool clicked;

				if (mouseIsOver(scrollbar.scrollThumb))
				{
					scrollbar.thumbDragging = true;
					scrollbar.scrollThumb.setFillColor(CONST::COLOR::SCROLLBAR::SCROLLTHUMB_HOLD);
				}

				//links
				for (size_t i = 0; i < items.size(); i++)
				{
					if (items[i]->downloaded)
					{
						if (mouseIsOver(items[i]->removeButton))
						{
							ModalOptions modOptions;
							modOptions.title = "Confirm Deletion";
							modOptions.text = "Delete \"" + items[i]->itemName + "\"?";
							std::vector<std::string> modaloptions;
							modaloptions.push_back("No");
							modaloptions.push_back("Yes");
							modOptions.settings = modaloptions;

							Modal confirmDelete(modOptions);

							if (confirmDelete.returnCode == 0)
							{
								std::cout << "answer no" << std::endl;
							}
							else if (confirmDelete.returnCode == 1)
							{
								std::cout << "answer yes" << std::endl;

								threads.push_back(std::thread(&Item::deleteFiles, items[i]));

//								std::thread *newThread = new std::thread(&Item::deleteFiles, items[i]);
//								threads.push_back(newThread);

								clicked = true;
								break;
							}
						}
						else if (mouseIsOver(items[i]->redownloadButton))
						{
							// update files
//							helperThread = new std::thread(&Item::updateItem, items[i]);

//							std::thread *newThread = new std::thread(&Item::updateItem, items[i]);
//							threads.push_back(new std::thread(&Item::updateItem, items[i]));

							threads.push_back(std::thread(&Item::updateItem, items[i]));

							clicked = true;
							break;
						}
						else if (mouseIsOver(items[i]->launchButton))
						{
							items[i]->openItem();

							clicked = true;
							break;
						}
					}
					else
					{
						if (mouseIsOver(items[i]->downloadButton))
						{
							// download files
							//helperThread = new std::thread(&Item::download, items[i]);

							threads.push_back(std::thread(&Item::download, items[i]));

//							std::thread *newThread = new std::thread(&Item::updateItem, items[i]);
//							threads.push_back(newThread);

							clicked = true;
							break;
						}
					}
				}

				//links
				for (size_t i = 0; i < links.size(); i++)
				{
					if (mouseIsOver(links[i]->linkText) || mouseIsOver(links[i]->followLinkButton))
					{
						// follow link
						links[i]->follow();
					}
				}
			}
			else if (event.key.code == sf::Mouse::Button::Right)
			{
				app->ChangeState(HomeState::Instance());
			}
		}
		else if (event.type == sf::Event::EventType::MouseButtonReleased)
		{
			if (scrollbar.thumbDragging)
			{
				scrollbar.thumbDragging = false;
				scrollbar.scrollThumb.setFillColor(CONST::COLOR::SCROLLBAR::SCROLLTHUMB_HOVER);
			}
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::R && !helperRunning)
			{
				links.clear();
				items.clear();

				helperThread = new std::thread(&AppListState::loadApps, this);
				helperDone = false;
				helperRunning = true;

				cardScroller->setCenter(cardScroller->getSize().x / 2, cardScroller->getSize().y / 2);
			}
		}
	}
}

void AppListState::Update()
{

}

void AppListState::Draw()
{
	app->window->clear(CONST::COLOR::BACKGROUND);

	for (size_t i = 0; i < threads.size(); i++)
	{
		if (threads[i].joinable())
		{
			std::cout << "joining" << std::endl;

			threads[i].detach();
			threads.erase(threads.begin() + i);
		}
	}

	//scrollable
	app->window->setView(*cardScroller);
	for (size_t i = 0; i < items.size(); i++)
		items[i]->draw();

	if (!links.empty())
		for (size_t i = 0; i < links.size(); i++)
			links[i]->draw();

	//anchored
//	app->window->setView(app->window->getDefaultView());
	// HACK: don't do this over and over. why does it even change when we scroll? I don't understand!
	app->window->setView(*mainView);
	scrollbar.draw();

	app->window->display();
}

void AppListState::loadApps() // TOOD: this.
{
	bool comesAfterLink(false), comesAfterItem(false);
	std::string line; // each line of index.dat;
	std::cout << std::endl;

	std::ifstream readIndex(".\\" + CONST::DIR::BASE + "\\index.dat", std::ios::in);
	int loopi(0);
	while (std::getline(readIndex, line))
	{
		std::cout << loopi << " - " << line << std::endl;

		if (line[0] == 'l' && line[1] == ':') // is a link
		{
			if (comesAfterItem)
			{
				std::cout << "(link after item)" << std::endl;

				Link* newLink = new Link(line, app->window, items.back()->cardShape.getPosition().y + 65); // we don't check to make sure this isn't empty, because we know there's an item before it.
				links.push_back(newLink);
			}
			else // after a link or first of  either
			{
				Link* newLink;

				if (links.empty())
				{
					newLink = new Link(line, app->window, 28);

					std::cout << "(link not after item, first link)" << std::endl;
				}
				else // not the first link
				{
					newLink = new Link(line, app->window, links.back()->cardShape.getPosition().y + 48);

					std::cout << "(link after link, not after item)" << std::endl;
				}

				links.push_back(newLink);
				std::cout << std::endl;
			}

			comesAfterItem = false;
			comesAfterLink = true;
		}
		else // is not a link
		{
			if (comesAfterLink)
			{
				std::cout << "(item after link)" << std::endl;

				Item* newItem = new Item(line, app->window, links.back()->cardShape.getPosition().y + 65);
				items.push_back(newItem);
				std::cout << std::endl;
			}
			else // not after a link
			{
				std::cout << "(item not after link)" << std::endl;

				Item* newItem;

				if (items.empty())
					newItem = new Item(line, app->window, 46);
				else
					newItem = new Item(line, app->window, items.back()->cardShape.getPosition().y + items.back()->totalHeight /* PADDING */);

				items.push_back(newItem);
				std::cout << std::endl;
			}

			comesAfterItem = true;
			comesAfterLink = false;
		}

		updateScrollThumb();
		loopi += 1;
	}

	helperDone = true;
	helperRunning = false;
}

void AppListState::updateScrollThumb()
{
	// set the scrollbar size
	float contentHeight(0);
	for (size_t i = 0; i < items.size(); i++)
	{
		contentHeight += items[i]->totalHeight;
	}

	for (size_t i = 0; i < links.size(); i++)
	{
		contentHeight += links[i]->totalHeight;
	}

	scrollbar.update(contentHeight, cardScroller->getSize().y);
}

bool AppListState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}

bool AppListState::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window), *cardScroller)))
		return true;
	else
		return false;
}