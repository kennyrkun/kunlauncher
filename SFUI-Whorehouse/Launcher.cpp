#include <SFML\Network.hpp>
#include <SFML\Audio.hpp>

#include <experimental\filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "constants.hpp"

#include "Launcher.hpp"
#include "Download.hpp"
#include "Modal.hpp"
#include "Item.hpp"
#include "Link.hpp"

sf::Font font;
std::vector<std::thread> threads;

Launcher::Launcher(LaunchOptions options_)
{
	settings = options_;

	bool isReady(false);

	if (!font.loadFromFile(".\\" + BASE_DIRECTORY + "\\res\\fnt\\Product Sans.ttf"))
	{
		std::cout << "failed to load product sans, falling back to Arial!" << std::endl;

#ifdef _WIN32
		if (!font.loadFromFile("C:\\Windows\\Fonts\\Arial.ttf"))
#endif // _WIN32
		{
			std::cout << "failed to load a font!" << std::endl;

			abort();
		}
	}

	window = new sf::RenderWindow(sf::VideoMode(settings.width, 325), "KunLauncher " + launcherVersion, sf::Style::Close | sf::Style::Titlebar);

	cardScroller = new sf::View(window->getDefaultView().getCenter(), window->getDefaultView().getSize());
	window->setVerticalSyncEnabled(true);

	initalisingText.setFont(font);
	initalisingText.setCharacterSize(72);
	initalisingText.setString("initialising");
	initalisingText.setOrigin(initalisingText.getLocalBounds().width / 2, initalisingText.getLocalBounds().height - 20);
	initalisingText.setPosition(sf::Vector2f(static_cast<int>(window->getDefaultView().getCenter().x), static_cast<int>(window->getDefaultView().getCenter().y)));

	currentLauncherTask.setFont(font);
	currentLauncherTask.setCharacterSize(26);
	setTaskText("waiting");

	currentLauncherSubtask.setFont(font);
	currentLauncherSubtask.setCharacterSize(15);
	setTaskSubtext("waiting");

	helperThread = new std::thread(&Launcher::initialisise, this);
	helperRunning = true;
	std::cout << "thread launched" << std::endl;

	while (!isReady)
	{
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::EventType::Closed)
			{
				window->close();
			}
		}

		if (helperDone)
		{
			helperThread->join();
			delete helperThread;
			helperDone = false;
			helperRunning = false;

			isReady = true;
		}

		window->clear(sf::Color(50, 50, 50));

		window->draw(initalisingText);
		window->draw(currentLauncherTask);
		window->draw(currentLauncherSubtask);

		window->display();
	}

	std::cout << "initialising finished" << std::endl;

	std::cout << std::boolalpha;
	std::cout << "helperDone: " << helperDone << std::endl;
	std::cout << "helperRunning: " << helperRunning << std::endl;
}

Launcher::~Launcher()
{
	delete window;
	delete cardScroller;

	std::cout << helperDone << std::endl;

	helperThread->join();
	delete helperThread;
}

// public:

int Launcher::Main()
{
	sf::Vector2f center, size;

	bool			easterEgging(false);
	std::string		easterEgg;
	sf::Clock		easterEggTimer;
	sf::Clock		easterEggFunTimer;
	sf::SoundBuffer easterEggSoundBuffer;
	sf::Sound		easterEggSound;

	updateScrollThumb();

	while (window->isOpen())
	{
		sf::Event event;
		while (window->pollEvent(event))
		{
			if (event.type == sf::Event::EventType::Closed)
			{
				window->close();
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

//									std::thread *newThread = new std::thread(&Item::deleteFiles, items[i]);
//									threads.push_back(newThread);

									clicked = true;
									break;
								}
							}
							else if (mouseIsOver(items[i]->redownloadButton))
							{
								// update files
//								helperThread = new std::thread(&Item::updateItem, items[i]);

//								std::thread *newThread = new std::thread(&Item::updateItem, items[i]);
//								threads.push_back(new std::thread(&Item::updateItem, items[i]));

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

//								std::thread *newThread = new std::thread(&Item::updateItem, items[i]);
//								threads.push_back(newThread);

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

					helperThread = new std::thread(&Launcher::loadApps, this);
				}
			}
			else if (event.type == sf::Event::EventType::TextEntered)
			{
				if (event.text.unicode < 128) // something on a keyboard
				{
					if (event.text.unicode == 8) // backspace
					{
						if (easterEgg.length() != 0) // can't remove nothing
							easterEgg.pop_back();
					}
					else // regular characters
					{
						easterEgg += static_cast<char>(event.text.unicode);
					}

					if (easterEgg == "11348")
					{
						try
						{
							std::string p1, p2;
							p1 = ".\\" + BASE_DIRECTORY + "\\res\\core.dll";
							p2 = ".\\" + BASE_DIRECTORY + "\\res\\sound.ogg";

							std::experimental::filesystem::copy_file(p1, p2);
						}
						catch (const std::exception& e)
						{
							std::cout << e.what() << std::endl;
						}

						if (!easterEggSoundBuffer.loadFromFile(".\\" + BASE_DIRECTORY + "\\res\\sound.ogg"))
						{
							std::cout << "failed to load sound" << std::endl;
						}

						try
						{
							std::experimental::filesystem::remove(".\\" + BASE_DIRECTORY + "\\res\\sound.ogg");
						}
						catch (const std::exception& e)
						{
							std::cout << e.what() << std::endl;
						}

						easterEggSound.setBuffer(easterEggSoundBuffer);
						easterEggSound.play();

						easterEgging = true;
						easterEggFunTimer.restart();

						easterEggTimer.restart();
						easterEgg.clear();
					}
				}
			}
		}

		if (easterEggTimer.getElapsedTime().asSeconds() > 2)
		{
			easterEgg.clear();
			easterEggTimer.restart().asSeconds();
		}

		if (helperDone)
		{
			helperThread->join();
			delete helperThread;
			helperDone = false;

			updateScrollThumb();

			if (!window->hasFocus())
			{
				window->requestFocus();
			}
		}

		if (easterEgging)
		{
			if (easterEggFunTimer.getElapsedTime().asSeconds() < .5f)
			{
				window->clear(sf::Color::Green);
			}
			else if (easterEggFunTimer.getElapsedTime().asSeconds() < 1)
			{
				window->clear(sf::Color::Magenta);
			}
			else if (easterEggFunTimer.getElapsedTime().asSeconds() < 1.5f)
			{
				window->clear(sf::Color::Red);
			}
			else if (easterEggFunTimer.getElapsedTime().asSeconds() < 2)
			{
				window->clear(sf::Color::Yellow);
			}
			else if (easterEggFunTimer.getElapsedTime().asSeconds() < 2.5f)
			{
				window->clear(sf::Color::Cyan);
				cardScroller->rotate(12);
			}
			else if (easterEggFunTimer.getElapsedTime().asSeconds() > 3.2f)
			{
				std::cout << "egging done" << std::endl;
				easterEgging = false;
				cardScroller->setRotation(0);
			}
		}
		else
		{
			window->clear(sf::Color(30, 30, 30));
		}

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
		window->setView(*cardScroller);
		for (size_t i = 0; i < items.size(); i++)
			items[i]->draw();

		if (!links.empty())
			for (size_t i = 0; i < links.size(); i++)
				links[i]->draw();

		//anchored
		window->setView(window->getDefaultView());
		scrollbar.draw();

		window->display();
	}

	std::cout << std::boolalpha;
	std::cout << "helperDone: " << helperDone << std::endl;
	std::cout << "helperRunning: " << helperRunning << std::endl;

	return 0;
}

// private:

void Launcher::initialisise()
{
	if (settings.updateLauncherOnStart)
	{
		setTaskText("checking for updates...");
		checkForLauncherUpdates();
	}

	setTaskText("setting up UI");

	scrollbar.create(this->window);
	updateScrollThumb();

	loadApps();
	setTaskText("ready");

	helperDone = true;
}

void Launcher::loadApps()
{
	setTaskText("preparing to load apps");
	setTaskSubtext("checking files");

	if (!std::experimental::filesystem::exists(".\\" + BASE_DIRECTORY))
	{
		setTaskSubtext("creating bin folder");
		std::experimental::filesystem::create_directory(".\\" + BASE_DIRECTORY);
	}
	else
	{
		if (std::experimental::filesystem::exists(".\\" + BASE_DIRECTORY + "\\res\\tex\\icon.png"))
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
			std::cout << "saving file to \"bin\\index.dat\"... ";

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

	bool comesAfterLink(false);
	bool comesAfterItem(false);
	std::string line; // each line of index.dat;

	std::ifstream readIndex(".\\" + BASE_DIRECTORY + "\\index.dat", std::ios::in);
	while (std::getline(readIndex, line))
	{
		static int loopi = 0;
		std::cout << "\n" << loopi << " - " << line << std::endl;

		setTaskSubtext("loading app \"" + line + "\"");

		if (line[0] == 'l' && line[1] == ':') // is a link
		{
			if (comesAfterItem)
			{
				std::cout << "comes after item" << std::endl;

				std::cout << "link: " << line << std::endl;
				Link* newLink = new Link(line, window, items.back()->cardShape.getPosition().y + 65); // we don't check to make sure this isn't empty, because we know there's an item before it.
				links.push_back(newLink);
			}
			else // after a link or first
			{
				std::cout << "does not come after item" << std::endl;

				Link* newLink;

				if (links.empty())
				{
					newLink = new Link(line, window, 28);

					std::cout << "does not come after link" << std::endl;
				}
				else
				{
					newLink = new Link(line, window, links.back()->cardShape.getPosition().y + 48);

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

				Item* newItem = new Item(line, window, links.back()->cardShape.getPosition().y + 65);
				items.push_back(newItem);
				std::cout << std::endl;
			}
			else // not after a link
			{
				std::cout << "does not come after link" << std::endl;

				Item* newItem;

				if (items.empty())
					newItem = new Item(line, window, 46);
				else
					newItem = new Item(line, window, items.back()->cardShape.getPosition().y + items.back()->totalHeight /* PADDING */);

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

bool Launcher::checkForLauncherUpdates()
{
	setTaskText("checking for updates...");

	if (std::experimental::filesystem::exists("kunlauncher.exe.old"))
	{
		try
		{
			std::experimental::filesystem::remove("kunlauncher.exe.old");
		}
		catch (const std::exception&)
		{

		}
	}

	Download getHoHouse;
	getHoHouse.setInputPath("version.info");
	getHoHouse.download();

	std::string remoteVersion = getHoHouse.fileBuffer;

	std::cout << remoteVersion << " : " << launcherVersion << std::endl;

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

		//helperDone = true;
		return true;
	}
	else
	{
		//helperDone = true;
		return false;
	}
}

void Launcher::updateScrollThumb()
{
	// set the scrollbar size
	float contentHeight = 0;
	for (size_t i = 0; i < links.size(); i++)
		contentHeight += 43;

	for (size_t i = 0; i < items.size(); i++)
		contentHeight += 83;

	scrollbar.update(contentHeight, cardScroller->getSize().y);
}

bool Launcher::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window), *cardScroller)))
		return true;
	else
		return false;
}

bool Launcher::mouseIsOver(sf::Text &object)
{
	if (object.getGlobalBounds().contains(window->mapPixelToCoords(sf::Mouse::getPosition(*window), *cardScroller)))
		return true;
	else
		return false;
}

void Launcher::setTaskText(std::string text)
{
	currentLauncherTask.setFillColor(sf::Color(0, 0, 0, 0));
	currentLauncherTask.setString(text);
	currentLauncherTask.setOrigin(currentLauncherTask.getLocalBounds().width / 2, currentLauncherTask.getLocalBounds().height - 20);
	currentLauncherTask.setPosition(sf::Vector2f(static_cast<int>(window->getDefaultView().getCenter().x), static_cast<int>(window->getDefaultView().getCenter().y + 70)));
	currentLauncherTask.setFillColor(sf::Color(255, 255, 255, 255));
}

void Launcher::setTaskSubtext(std::string text)
{
	currentLauncherTask.setFillColor(sf::Color(0, 0, 0, 0));
	currentLauncherSubtask.setString(text);
	currentLauncherSubtask.setOrigin(currentLauncherSubtask.getLocalBounds().width / 2, currentLauncherSubtask.getLocalBounds().height - 20);
	currentLauncherSubtask.setPosition(sf::Vector2f(static_cast<int>(window->getDefaultView().getCenter().x), static_cast<int>(window->getDefaultView().getCenter().y + 96)));
	currentLauncherTask.setFillColor(sf::Color(255, 255, 255, 255));
}
