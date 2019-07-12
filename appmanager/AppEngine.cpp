#include "AppEngine.hpp"
#include "../SFUI-Whorehouse/AppState.hpp"

#include "Globals.hpp"
#include "Download.hpp"

#include <SFUI/Theme.hpp>

#include <iostream>
#include <ctime>
#include <experimental/filesystem>

// TODO: load program icon

namespace fs = std::experimental::filesystem;

void AppEngine::Init(std::string title_, AppSettings settings_)
{
	std::cout << "AppEngine Init" << std::endl;

	settings = settings_;

	window = new sf::RenderWindow;
	window->setVerticalSyncEnabled(settings.window.verticalSync);

	running = true;
}

void AppEngine::Cleanup()
{
	std::cout << "Cleaning up AppEngine." << std::endl;
	
	for (size_t i = 0; i < states.size(); i++)
		PopState();

	window->close();
	delete window;

	Download clearCache;
	clearCache.clearCache();

	std::cout << "AppEngine cleaned up." << std::endl;
}

void AppEngine::ChangeState(AppState* state)
{
	queuedEvents.push_back(std::pair<EventType, AppState*>(EventType::ChangeState, state));
}

void AppEngine::PushState(AppState* state)
{
	queuedEvents.push_back(std::pair<EventType, AppState*>(EventType::PushState, state));
}

void AppEngine::PopState()
{
	queuedEvents.push_back(std::pair<EventType, AppState*>(EventType::PopState, nullptr));
}

void AppEngine::HandleEvents()
{
	if (running && !states.empty())
		states.back()->HandleEvents();

	for (size_t i = 0; i < queuedEvents.size(); i++)
	{
		if (queuedEvents[i].first == EventType::ChangeState)
		{
			if (!states.empty())
			{
				states.back()->Cleanup();

				delete states.back();
				states.pop_back();
			}

			states.push_back(queuedEvents[i].second);
			states.back()->Init(this);
		}
		else if (queuedEvents[i].first == EventType::PushState)
		{
			if (!states.empty())
				states.back()->Pause();

			states.push_back(queuedEvents[i].second);
			states.back()->Init(this);
		}
		else if (queuedEvents[i].first == EventType::PopState)
		{
			if (!states.empty())
			{
				states.back()->Cleanup();

				delete states.back();
				states.pop_back();
			}

			if (!states.empty())
				states.back()->Resume();
		}
		else if (queuedEvents[i].first == EventType::Quit)
		{
			Quit();
		}

		// this might break things, but I'm not sure.
		// I am confident it will work.
		queuedEvents.pop_back();
	}
}

void AppEngine::Update()
{
	if (running)
		states.back()->Update();
}

void AppEngine::Draw()
{
	if (running)
		states.back()->Draw();
}

/*
void AppEngine::UpdateViewSize(const sf::Vector2f& size)
{
	std::cout << "new width: " << size.x << std::endl;
	std::cout << "new height: " << size.y << std::endl;

	sf::Vector2u newSize(size.x, size.y);

	//TODO: define max size somewhere
	if (newSize.x >= 525 && newSize.y >= 325)
	{
		sf::FloatRect visibleArea(0, 0, size.x, size.y);
		window->setView(sf::View(visibleArea));
	}
	else
	{
		if (size.x <= settings.window.width)
			newSize.x = settings.window.width;

		if (size.x <= settings.window.height)
			newSize.y = settings.window.height;

		window->setSize(newSize);
	}
}
*/

// https://stackoverflow.com/questions/997946/how-to-get-current-time-and-date-in-c/10467633#10467633
const std::string AppEngine::currentDateTime() 
{
	time_t     now = time(0);
	struct tm  timeinfo;
	char       buf[80];
	localtime_s(&timeinfo, &now);
	strftime(buf, sizeof(buf), "%F.%H-%M-%S", &timeinfo);

	return buf;
}

void AppEngine::Quit()
{
	std::cout << "Shutting down AppEngine" << std::endl;

	running = false;

	for (size_t i = 0; i < states.size(); i++)
	{
		std::cout << "cleaning up state " << i << std::endl;

		states.back()->Cleanup();
		delete states.back();
		states.pop_back();
	}

	states.clear();
}

std::vector<std::string> AppEngine::get_directories(const std::string& s)
{
	std::vector<std::string> r;
	for (auto& p : fs::directory_iterator(s))
		if (p.status().type() == fs::file_type::directory)
			r.push_back(p.path().string().substr(s.length(), p.path().string().length()));
	return r;
}

bool AppEngine::updateAppIndex()
{
	std::cout << "[AppEngine] redownloading app index." << std::endl;

	Download getNewIndex;
	getNewIndex.setInput("./" + GBL::WEB::APPS + "/index.dat");
	getNewIndex.setOutputDir(GBL::DIR::appcache);
	getNewIndex.setOutputFilename("/index.dat");

	int status = getNewIndex.download();

	if (status == Download::Status::Ok)
	{
		getNewIndex.save();
		return true;
	}
	else
	{
		std::cerr << "[AppEngine] failed to download new index" << std::endl;
		return false;
	}
}

void AppEngine::drawInformationPanel(std::string text)
{
	sf::RectangleShape shape(sf::Vector2f(window->getSize().x, window->getSize().y));
	shape.setFillColor(sf::Color(0, 0, 0, 100));

	sf::RectangleShape middle(sf::Vector2f(window->getSize().x, 100));
	middle.setFillColor(SFUI::Theme::windowBgColor);
	middle.setPosition(sf::Vector2f(0, (window->getSize().y / 2) - 50));

	sf::Text t;
	t.setFont(SFUI::Theme::getFont());
	t.setCharacterSize(48);
	t.setString(text);
	t.setPosition(sf::Vector2f(25, middle.getPosition().y + 18));

	window->draw(shape);
	window->draw(middle);
	window->draw(t);
	window->display();
}
