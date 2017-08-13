#include "AppEngine.hpp"
#include "AppState.hpp"
#include "AppListState.hpp"
#include "AppInfoState.hpp"

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

sf::Font font;
std::vector<std::thread> threads;

AppListState::AppListState(AppEngine2* app_)
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

	cardScroller = new sf::View(app->window->getDefaultView().getCenter(), app->window->getDefaultView().getSize());
	app->window->setVerticalSyncEnabled(true);

	initalisingText.setFont(font);
	initalisingText.setCharacterSize(72);
	initalisingText.setString("initialising");
	initalisingText.setOrigin(initalisingText.getLocalBounds().width / 2, initalisingText.getLocalBounds().height - 20);
	initalisingText.setPosition(sf::Vector2f(static_cast<int>(app->window->getDefaultView().getCenter().x), static_cast<int>(app->window->getDefaultView().getCenter().y)));

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
			{
				app->window->close();
			}
		}

		if (helperDone)
		{
			helperThread->join();
			delete helperThread;
			helperDone = true;
			helperRunning = false;

			isReady = true;
		}

		app->window->clear(sf::Color(50, 50, 50));

		app->window->draw(initalisingText);
		app->window->draw(currentLauncherTask);
		app->window->draw(currentLauncherSubtask);

		app->window->display();
	}

	std::cout << "initialising finished" << std::endl;

	std::cout << std::boolalpha;
	std::cout << "helperDone: " << helperDone << std::endl;
	std::cout << "helperRunning: " << helperRunning << std::endl;

	updateScrollThumb();
}

AppListState::~AppListState()
{
	delete cardScroller;
	delete app;

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
		else if (event.type == sf::Event::EventType::MouseWheelMoved) // thanks sfconsole
		{
			if (event.mouseWheel.delta < 0) // up
			{
				if ((cardScroller->getCenter().y - cardScroller->getSize().y) < scrollbar.scrollJumpMultiplier) // top of the thing
				{
					cardScroller->move(0, scrollbar.scrollJump);
					scrollbar.moveThumbUp();
				}
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

				//links
				for (size_t i = 0; i < items.size(); i++)
				{
					if (items[i]->downloaded)
					{
						if (mouseIsOver(items[i]->removeButton))
						{
							ModalOptions modOptions;
							modOptions.title = "Confirm Deletion";
							modOptions.text = "Delete " + items[i]->itemName;
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
						else if (false)
						{
							app->PushState(new AppInfoState2(this->app, app->states.size()));
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
					if (mouseIsOver(links[i]->linkText) || mouseIsOver(links[i]->followLink))
					{
						// follow link
						links[i]->follow();
					}
				}
			}
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::R)
			{
				links.clear();
				items.clear();

				helperThread = new std::thread(&AppListState::loadApps, this);
			}
		}
	}
}

void AppListState::Update()
{

}

void AppListState::Draw()
{
	app->window->clear(sf::Color(50, 50, 50));

	items[0]->name.setString(std::to_string(threads.size()));

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
	app->window->setView(app->window->getDefaultView());
	scrollbar.draw();

	app->window->display();
}

void AppListState::initialisise()
{
	if (settings.updateLauncherOnStart)
	{
		setTaskText("checking for updates...");
		checkForLauncherUpdates();
	}

	setTaskText("setting up UI");

	scrollbar.create(app->window);
	updateScrollThumb();

	loadApps();
	setTaskText("ready");

	helperDone = true;
}

void AppListState::loadApps()
{
	setTaskText("loading apps");
	setTaskSubtext("checking files");

	if (!std::experimental::filesystem::exists(".\\" + BASE_DIRECTORY))
	{
		setTaskSubtext("creating bin folder");
		std::experimental::filesystem::create_directory(".\\" + BASE_DIRECTORY);
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

	setTaskText("loading apps (this may take a while)");
	setTaskSubtext("");

	bool comesAfterLink(false), comesAfterItem(false);
	std::string line; // each line of index.dat;

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
				std::cout << "comes after item" << std::endl;

				std::cout << "link: " << line << std::endl;
				Link* newLink = new Link(line, app->window, items.back()->cardShape.getPosition().y + 65); // we don't check to make sure this isn't empty, because we know there's an item before it.
				links.push_back(newLink);
			}
			else // after a link or first
			{
				std::cout << "does not come after item" << std::endl;

				Link* newLink;

				if (links.empty())
				{
					newLink = new Link(line, app->window, 28);

					std::cout << "does not come after link" << std::endl;
				}
				else
				{
					newLink = new Link(line, app->window, links.back()->cardShape.getPosition().y + 48);

					std::cout << "comes after link" << std::endl;
				}

				std::cout << "link2: " << line << std::endl;
				links.push_back(newLink);
			}

			comesAfterItem = false;
			comesAfterLink = true;
		}
		else // is not a link
		{
			if (comesAfterLink)
			{
				std::cout << "comes after link" << std::endl;

				Item* newItem = new Item(line, app->window, links.back()->cardShape.getPosition().y + 65);
				items.push_back(newItem);
				std::cout << std::endl;
			}
			else // not after a link
			{
				std::cout << "does not come after link" << std::endl;

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
	{
		setTaskText("updating launcher");

		std::cout << "launcher is out of date (current: " << launcherVersion << "; remote: " << remoteVersion << ")" << std::endl;
		setTaskSubtext("downloading updated launcher");

		Download getNewWhorehouse;
		getNewWhorehouse.setInputPath("latest.noexe");
		getNewWhorehouse.setOutputDir(".\\");
		getNewWhorehouse.setOutputFile("kunlauncher.exe");
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

		helperDone = true;
		return true;
	}
	else
	{
		//helperDone = true;
		return false;
	}
}

void AppListState::updateScrollThumb()
{
	// set the scrollbar size
	float contentHeight = 0;
	for (size_t i = 0; i < links.size(); i++)
		contentHeight += 43;

	for (size_t i = 0; i < items.size(); i++)
		contentHeight += 83;

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
	std::cout << "setting text to \"" << text << "\"" << std::endl;

	currentLauncherTask.setString(text);
	currentLauncherTask.setOrigin(currentLauncherTask.getLocalBounds().width / 2, currentLauncherTask.getLocalBounds().height - 20);
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(app->window->getDefaultView().getCenter().x), static_cast<int>(app->window->getDefaultView().getCenter().y + 70)));
}

void AppListState::setTaskSubtext(std::string text)
{
	std::cout << "setting subtext to \"" << text << "\"" << std::endl;

	currentLauncherSubtask.setString(text);
	currentLauncherSubtask.setOrigin(currentLauncherSubtask.getLocalBounds().width / 2, currentLauncherSubtask.getLocalBounds().height - 20);
	currentLauncherSubtask.setPosition(sf::Vector2f(static_cast<int>(app->window->getDefaultView().getCenter().x), static_cast<int>(app->window->getDefaultView().getCenter().y + 96)));
}
