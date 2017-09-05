#include "AppEngine.hpp"
#include "AppState.hpp"
#include "AppListState.hpp"

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

sf::Font font;
std::vector<std::thread> threads;

void AppListState::Init(AppEngine* app_)
{
	std::cout << "AppListState Init" << std::endl;

	app = app_;

	bool isReady(false);

	if (!font.loadFromFile(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + FONT_DIRECTORY + "\\Product Sans.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << std::endl;

		if (!font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
		{
			std::cout << "failed to load a font!" << std::endl;

			abort();
		}
	}

	app->window->setTitle("KunLauncher " + launcherVersion);

	cardScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	mainView = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	app->window->setVerticalSyncEnabled(true);

	initalisingText.setFont(font); 
	initalisingText.setCharacterSize(72);
	initalisingText.setString("initialising");
	initalisingText.setOrigin(initalisingText.getLocalBounds().width / 2, initalisingText.getLocalBounds().height - 20);
	initalisingText.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y)));

	currentLauncherTask.setFont(font);
	currentLauncherTask.setCharacterSize(26);
	setTaskText("waiting");

	currentLauncherSubtask.setFont(font);
	currentLauncherSubtask.setCharacterSize(15);
	setTaskSubtext("waiting");
	
	helperThread = new std::thread(&AppListState::initialisise, this);
	helperRunning = true;
	std::cout << "thread launched" << std::endl;

	while (!isReady)
	{
		sf::Event event;
		while (app->window->pollEvent(event))
		{
			if (event.type == sf::Event::EventType::Closed)
				app->Quit();
		}

		if (helperDone)
		{
			std::cout << "helper is done, joining" << std::endl;

			helperThread->join();
			delete helperThread;
			helperDone = true;
			helperRunning = false;

			isReady = true;
		}

		app->window->clear(CONST_COLOURS::BACKGROUND);

		app->window->draw(initalisingText);
		app->window->draw(currentLauncherTask);
		app->window->draw(currentLauncherSubtask);

		app->window->display();
	}

	std::cout << "initialising finished" << std::endl;

	std::cout << std::boolalpha;
	std::cout << "helperDone: " << helperDone << std::endl;
	std::cout << "helperRunning: " << helperRunning << std::endl;

	std::cout << "links: " << links.size() << std::endl;
	std::cout << "items: " << items.size() << std::endl;
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
					scrollbar.scrollThumb.setFillColor(CONST_COLOURS::SCROLLBAR::SCROLLTHUMB_HOLD);
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
		}
		else if (event.type == sf::Event::EventType::MouseButtonReleased)
		{
			if (scrollbar.thumbDragging)
			{
				scrollbar.thumbDragging = false;
				scrollbar.scrollThumb.setFillColor(CONST_COLOURS::SCROLLBAR::SCROLLTHUMB_HOVER);
			}
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::R)
			{
				links.clear();
				items.clear();

				helperThread = new std::thread(&AppListState::loadApps, this);

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
	app->window->clear(CONST_COLOURS::BACKGROUND);

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

void AppListState::initialisise()
{
	if (!std::experimental::filesystem::exists(".\\" + BASE_DIRECTORY))
	{
		setTaskSubtext("creating bin folder");
		std::experimental::filesystem::create_directory(".\\" + BASE_DIRECTORY);

		settings.updateItemsOnStart = true;
	}
	else
	{
		if (std::experimental::filesystem::exists(".\\" + BASE_DIRECTORY + "\\" + RESOURCE_DIRECTORY + "\\" + TEXTURE_DIRECTORY + "\\icon.png"))
		{
			//	sf::Image icon;
			//	icon.loadFromFile(".\\" + BASE_FOLDER + "\\res\\tex\\icon.png");
			//	window->setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
		}
	}

	setTaskSubtext("checking if index file exists");
	if (!std::experimental::filesystem::exists(".\\" + BASE_DIRECTORY + "\\index.dat"))
	{
		setTaskSubtext("creating empty index files");
		std::ofstream createIndex(".\\" + BASE_DIRECTORY + "\\index.dat");
		createIndex.close();

		settings.updateItemsOnStart = true;
	}

	//TODO: this will stop items from updating if it's disabled
	if (settings.updateItemsOnStart)
	{
		setTaskSubtext("connecting to file server");

		// download the index file (or at least store it)
		sf::Http http("kunlauncher.000webhostapp.com/");
		sf::Http::Request request("/index.dat", sf::Http::Request::Get);
		sf::Http::Response response = http.sendRequest(request);

		int fileSize = response.getBody().size();

		// if the index file on the server has a different filesize than the one we've got, download it
		setTaskSubtext("gathering apps list");
		if (std::experimental::filesystem::file_size(".\\" + BASE_DIRECTORY + "\\index.dat") != fileSize)
		{
			std::cout << "index file has been updated (difference of ";
			if (std::experimental::filesystem::file_size(".\\" + BASE_DIRECTORY + "\\index.dat") > fileSize)
			{
				std::cout << std::experimental::filesystem::file_size(".\\" + BASE_DIRECTORY + "\\index.dat") - fileSize << " bytes)" << std::endl;
			}
			else
			{
				std::cout << fileSize - std::experimental::filesystem::file_size(".\\" + BASE_DIRECTORY + "\\index.dat") << " bytes)" << std::endl;
			}

			setTaskSubtext("updating apps list");
			std::cout << "updating apps list" << std::endl;

			std::string fileContainer = response.getBody();
			std::ofstream downloadFile(".\\" + BASE_DIRECTORY + "\\index.dat", std::ios::out | std::ios::binary);
			std::cout << "saving file to \"" + BASE_DIRECTORY + "\\index.dat\"... ";

			for (int i = 0; i < fileSize; i++)
				downloadFile << fileContainer[i];
			downloadFile.close();

			if (downloadFile.fail())
				std::cout << "failed" << std::endl;
			else
				std::cout << "finished" << std::endl;

			std::cout << "index file is ready." << std::endl;
		}
	}


	if (settings.updateLauncherOnStart)
	{
		setTaskText("checking for updates...");
		
		if (checkForLauncherUpdates())
		{
			bool doUpdate = false;

			Download getHoHouse;
			getHoHouse.setInputPath("version.info");
			getHoHouse.download();

			if (getHoHouse.fileBuffer != launcherVersion)
			{
				ModalOptions modOptions;

				if (getHoHouse.fileBuffer.find("500 Internal Server Error") != std::string::npos)
				{
					modOptions.title = "fuckin peice of shit is broken again";
					modOptions.text = "server fucked up, committing suicide.";
					modOptions.settings = { "reopen the launcher and hope to god it doesn't break again" };
				}
				else
				{
					modOptions.title = "Update Available";
					modOptions.text = "Version " + getHoHouse.fileBuffer + " is available, would you like to update?";
					modOptions.settings = { "Yes", "No" };
				}

				Modal doYouWannaUpdate(modOptions);

				switch (doYouWannaUpdate.returnCode)
				{
				case 0:
					std::cout << "yes, update now." << std::endl;
					doUpdate = true;
					break;

				case 1:
					std::cout << "don't update now" << std::endl;
					doUpdate = false;
					break;

				default:
					break;
				}
			}

			if (doUpdate)
			{
				std::string remoteVersion = updateLauncher(); // feels kinda hacky

				ModalOptions modOptions;
				modOptions.text = "Launcher updated";

				if (remoteVersion.find("500 Internal Server Error") != std::string::npos)
				{
					modOptions.text = "server fucked up, committing suicide.";
					modOptions.settings = { "reopen the launcher and hope to god it doesn't break again" };
				}
				else
				{
					modOptions.text = "Launcher updated to v" + remoteVersion + "! Restart it!";
					modOptions.settings = { "Restart Now", "Restart Later" };
				}

				Modal updateSuccessfulModal(modOptions);

				switch (updateSuccessfulModal.returnCode)
				{
				case 0:
					std::cout << "restarting now" << std::endl;
					exit(0); // TODO: shutdown properly
					break;

				case 1:
					std::cout << "restarting later" << std::endl;
					updateSuccessfulModal.close();

				default:
					break;
				}
			}
			else
			{
				std::cout << "updating skipped" << std::endl;
			}
		}
		else
		{
			std::cout << "no updates were found" << std::endl;
		}
	}
	else
	{
		std::cout << "skipping check for updates" << std::endl;
	}

	scrollbar.create(app->window);

	loadApps();
	setTaskText("ready");

	helperDone = true;
}

void AppListState::loadApps() // TOOD: this.
{
	setTaskText("loading apps");
	setTaskSubtext("checking files");

	setTaskText("loading apps (this may take a while)");
	setTaskSubtext("");

	bool comesAfterLink(false), comesAfterItem(false);
	std::string line; // each line of index.dat;
	std::cout << std::endl;

	std::ifstream readIndex(".\\" + BASE_DIRECTORY + "\\index.dat", std::ios::in);
	int loopi(0);
	while (std::getline(readIndex, line))
	{
		std::cout << loopi << " - " << line << std::endl;

		setTaskSubtext("loading app \"" + line + "\"");

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

	setTaskSubtext("");
	helperDone = true;
}

bool AppListState::checkForLauncherUpdates()
{
	setTaskText("checking for updates...");

	if (std::experimental::filesystem::exists("kunlauncher.exe.old"))
	{
		try
		{
			std::experimental::filesystem::remove("kunlauncher.exe.old");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}
	}

	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	std::string remoteVersion = getHoHouse.fileBuffer;

	std::cout << "r" << remoteVersion << " : " << "l" << launcherVersion << std::endl;

	if (remoteVersion != launcherVersion)
		return true;
	else
		return false;
}

std::string AppListState::updateLauncher()
{
	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	std::string remoteVersion = getHoHouse.fileBuffer;

	std::cout << "r" << remoteVersion << " : " << "l" << launcherVersion << std::endl;

	if (remoteVersion != launcherVersion)
	{
		setTaskText("updating launcher");

		std::cout << "launcher is out of date (current: " << launcherVersion << "; remote: " << remoteVersion << ")" << std::endl;
		setTaskSubtext("downloading updated launcher");

		Download getNewWhorehouse;
		getNewWhorehouse.setInputPath("latest.noexe");
		getNewWhorehouse.setOutputDir(".\\");
		getNewWhorehouse.setOutputFilename("kunlauncher.exe");
		getNewWhorehouse.download();

		setTaskSubtext("saving updated launcher");

		try
		{
			std::experimental::filesystem::rename("kunlauncher.exe", "kunlauncher.exe.old");
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
		}

		getNewWhorehouse.save();

		return remoteVersion;
	}
	else
	{
		return launcherVersion;
	}
}

void AppListState::updateScrollThumb()
{
	// set the scrollbar size
	float contentHeight(0);
	for (size_t i = 0; i < items.size(); i++)
	{
//		contentHeight += 83;
		contentHeight += items[i]->totalHeight;
//		contentHeight += 5;
	}

	for (size_t i = 0; i < links.size(); i++)
	{
//		contentHeight += 43;
		contentHeight += links[i]->totalHeight;
//		contentHeight += 5;
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

void AppListState::setTaskText(std::string text)
{
	currentLauncherTask.setString(text);
	currentLauncherTask.setOrigin(currentLauncherTask.getLocalBounds().width / 2, currentLauncherTask.getLocalBounds().height - 20);
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y + 70)));
}

void AppListState::setTaskSubtext(std::string text)
{
	currentLauncherSubtask.setString(text);
	currentLauncherSubtask.setOrigin(currentLauncherSubtask.getLocalBounds().width / 2.0f, currentLauncherSubtask.getLocalBounds().height - 20.0f);
	currentLauncherSubtask.setPosition(sf::Vector2f(static_cast<int>(app->window->getView().getCenter().x), static_cast<int>(app->window->getView().getCenter().y + 96)));
}
