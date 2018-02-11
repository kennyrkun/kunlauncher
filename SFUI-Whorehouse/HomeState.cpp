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

void wrap(sf::Text& target, const float width)
{
	std::cout << "wrapping string from " << width << std::endl;

	std::string str = target.getString();

	//	const float containerWidth = target.getCharacterSize();
	const float containerWidth = width;
	for (auto i = 0u; i < target.getString().getSize(); ++i)
	{
		if (target.findCharacterPos(i).x >= containerWidth)
		{
			//str.insert(str.rfind(' ', i), "\n");
			str.insert(i, "\n");
			target.setString(str);
		}
	}

	std::cout << "string wrapped" << std::endl;
}

// NAVSECTION

NavbarSection::NavbarSection(std::string str, int sectionNum) : str(str), sectionNum(sectionNum)
{
	std::cout << "creating navbar section \"" << str << "\" (" << sectionNum << ")" << std::endl;

	font.loadFromFile(GBL::DIR::fonts + "Arial.ttf");

	text.setFont(font);
	text.setString(str);
}

NavbarSection::~NavbarSection()
{
	std::cout << "destroying navbar section " << str << "(" << sectionNum << ")" << std::endl;
}

void NavbarSection::update()
{
}

// NAVBAR

Navbar::Navbar(sf::RenderWindow* window) : window(window)
{
	std::cout << "creating navbar" << std::endl;

	bar.setSize(sf::Vector2f(window->getSize().x, 40));
	bar.setFillColor(GBL::COLOR::PRIMARY);

	std::cout << "navbar ready" << std::endl;
}

Navbar::~Navbar()
{
	std::cout << "destroying navbar" << std::endl;

	std::cout << "destroying destroyed" << std::endl;
}

void Navbar::addSection(std::string text)
{
	std::cout << "adding section \"" << text << "\"" << std::endl;

	NavbarSection* newSection = new NavbarSection(text, sections.size());

	if (sections.empty())
		newSection->text.setPosition(sf::Vector2f(10, 0));
	else
		newSection->text.setPosition(sections.back()->text.getPosition().x + sections.back()->text.getLocalBounds().width + 34, 0);

//	if (newSection->text.getPosition())
//	{

//	}

	sections.push_back(newSection);
}

void Navbar::removeSection(int sectionNum)
{
	std::cout << "removing section " << sectionNum << std::endl;

	if (sections[sectionNum] == nullptr)
	{
		std::cout << "section " << sectionNum << " does not exist" << std::endl;
		return;
	}

	// I don't know if this actually works
	delete sections[sectionNum];
	sections.erase(std::remove(sections.begin(), sections.end(), sections[sectionNum]), sections.end());
}

void Navbar::HandleEvents(const sf::Event & event)
{
	if (event.type == sf::Event::EventType::Resized)
	{
		bar.setSize(sf::Vector2f(event.size.width, 40));
	}
}

void Navbar::Update()
{
}

void Navbar::Draw()
{
	window->draw(bar);

	for (size_t i = 0; i < sections.size(); i++)
		window->draw(sections[i]->text);
}

// NEWS

News::News(std::string titlestr, std::string textstr, sf::RenderWindow* window) : titlestr(titlestr), textstr(textstr), window(window)
{
	font.loadFromFile(GBL::DIR::fonts + "Arial.ttf");

	title.setFont(font);
	title.setCharacterSize(26);
	title.setString(titlestr);

	text.setFont(font);
	text.setCharacterSize(14);
	text.setString(textstr);

	divider.setSize(sf::Vector2f(window->getSize().x - 40, 2));

	wrap(text, window->getSize().x - 20);
}

News::~News()
{

}

void News::setPosition(const sf::Vector2f& pos)
{
	title.setPosition(pos);

	divider.setPosition(sf::Vector2f(title.getPosition().x + 10, title.getPosition().y + title.getLocalBounds().height + 15));

	text.setPosition(sf::Vector2f(title.getPosition().x, title.getPosition().y + title.getLocalBounds().height + 20));
}

sf::Vector2f News::getPosition()
{
	return title.getPosition();
}

float News::getLocalHeight()
{
	return (text.getPosition().y + text.getLocalBounds().height) - title.getPosition().y;
}

void News::HandleEvents(const sf::Event & event)
{
}

void News::Update()
{
}

void News::Draw()
{
	window->draw(title);
	window->draw(divider);
	window->draw(text);
}

// STATE

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

	sf::Vector2f center(app->window->getView().getCenter().x, app->window->getView().getCenter().y - 40);
	sf::Vector2f size(app->window->getView().getSize().x, app->window->getView().getSize().y - 40);
	viewScroller = new sf::View(app->window->getView().getCenter(), app->window->getView().getSize());
	scrollbar.create(app->window);

	loadNews();

	std::cout << "HomeState ready" << std::endl;
}

void HomeState::Cleanup()
{
	std::cout << "Cleaning up HomeState" << std::endl;

	if (helperRunning)
	{
		std::cout << "waiting on helper thread to finish" << std::endl;
		helperThread->join();
	}

	sections.clear();

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
			app->UpdateViewSize(sf::Vector2f(event.size.width, event.size.height));

			sf::Vector2u newSize(event.size.width, event.size.height);

			viewScroller->setSize(sf::Vector2f(event.size.width, event.size.height));
			viewScroller->setCenter(sf::Vector2f(viewScroller->getSize().x / 2, viewScroller->getSize().y / 2));

			divider.setSize(sf::Vector2f(event.size.width - 40, 2));

			updateScrollThumbSize();
		}
		else if (event.type == sf::Event::EventType::KeyPressed)
		{
			if (event.key.code == sf::Keyboard::Key::Escape)
			{
				app->PopState();
			}
		}
		else if (event.type == sf::Event::EventType::MouseButtonPressed)
		{
			if (event.mouseButton.button == sf::Mouse::Button::Left)
			{
				if (mouseIsOver(navbar->sections[1]->text))
					app->PushState(MyAppListState::Instance());
				else if (mouseIsOver(navbar->sections[2]->text))
					app->PushState(AllAppsListState::Instance());
				else if (mouseIsOver(navbar->sections[3]->text))
					app->PushState(SettingsState::Instance());
			}
		}
		else if (event.type == sf::Event::EventType::MouseWheelMoved)
		{
			if (event.mouseWheel.delta < 0) // down, or move items up
			{
				scrollbar.moveThumbDown();

				if (scrollerBottomPosition < scrollerMaxPosition)
					viewScroller->move(0, scrollbar.scrollJump);
				else
					std::cout << "cannot scroll view down (" << scrollerBottomPosition << " < " << scrollerMaxPosition << ")" << std::endl;

				if (scrollerBottomPosition > scrollerMaxPosition) // clamp cardScroller
				{
					std::cout << "cardScroller went too far down (" << scrollerBottomPosition - scrollerMaxPosition << "), clamping..." << std::endl;
					viewScroller->setCenter(viewScroller->getCenter().x, scrollerMaxPosition - viewScroller->getSize().y / 2 + 8);
				}

				updateScrollLimits();
			}
			else if (event.mouseWheel.delta > 0) // scroll up, or move items down
			{
				scrollbar.moveThumbUp();

				if (scrollerTopPosition > scrollerMinPosition)
					viewScroller->move(0, -scrollbar.scrollJump);
				else
					std::cout << "cannot scroll view up (" << scrollerTopPosition << " > " << scrollerMaxPosition << ")" << std::endl;

				if (scrollerTopPosition < scrollerMinPosition) // clamp cardScroller
				{
					std::cout << "cardScroller went too far up (" << scrollerMaxPosition - scrollerTopPosition << "), clamping..." << std::endl;
					viewScroller->setCenter(viewScroller->getCenter().x, scrollerMinPosition + viewScroller->getSize().y / 2);
				}

				updateScrollLimits();
			}
		}
	}
}

void HomeState::Update()
{
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

	app->window->setView(app->window->getDefaultView());
	app->window->draw(scrollbar);
	navbar->Draw();

	app->window->display();
}

void HomeState::loadNews()
{
	helperDone = false;
	helperRunning = true;
//	loadingApps = true;

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
	}

	std::cout << newses.size() << std::endl;

	readIndex.close();

	app->window->requestFocus();

	helperDone = true;
	helperRunning = false;
//	loadingApps = false;
}

void HomeState::updateScrollThumbSize()
{
	float contentHeight(0);
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
