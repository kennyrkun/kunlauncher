#include "AppEngine.hpp"
#include "NewsEditState.hpp"
#include "NewsUploadState.hpp"

#include "Globals.hpp"
#include "Download.hpp"
#include "../SFUI-Whorehouse/SettingsParser.hpp"

#include <SFUI/SFUI.hpp>

#include <iostream>
#include <fstream>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

enum MenuCallbacks
{
	Upload,
	Back
};

NewsEditState::NewsEditState(const std::string& newsToEdit) : newsToEdit(newsToEdit)
{
}

void NewsEditState::Init(AppEngine* app_)
{
	std::cout << "NewsEditState Init" << std::endl;
	app = app_;

	scrollbar.create(app->window);
	scrollbar.setTrackHeight(app->window->getSize().y);
	scrollbar.setPosition(sf::Vector2f(scrollbar.getPosition().x, 0));

	loadNewsContent(newsToEdit);

	std::cout << "creating menu" << std::endl;
	menu = new SFUI::Menu(*app->window);
	createMenu(*menu);

	std::cout << "NewsEditState ready." << std::endl;
}

void NewsEditState::Cleanup()
{
	std::cout << "Cleaning up NewsEditState" << std::endl;

	delete menu;

	std::cout << "Cleaned up NewsEditState." << std::endl;
}

void NewsEditState::Pause()
{
	std::cout << "NewsEditState paused" << std::endl;
}

void NewsEditState::Resume()
{
	std::cout << "NewsEditState resumed" << std::endl;
}

void NewsEditState::HandleEvents()
{
	sf::Event event;

	while (app->window->pollEvent(event))
	{
		if (event.type == sf::Event::EventType::Closed)
		{
			std::cout << "quitting" << std::endl;

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
				app->window->setView(sf::View(visibleArea));
			}
			else
			{
				if (event.size.width <= 525)
					newSize.x = 525;

				if (event.size.height <= 325)
					newSize.y = 325;

				app->window->setSize(newSize);
			}

			scrollbar.setTrackHeight(app->window->getSize().y);
			scrollbar.setPosition(sf::Vector2f(app->window->getSize().x, 0));
			updateScrollThumbSize();

			menu->setPosition(sf::Vector2f(10, 10));
		}

		int id = menu->onEvent(event);
		switch (id)
		{
		case MenuCallbacks::Upload:
			overwriteOldNewsContent();
			app->PushState(new NewsUploadState);
			break;
		case MenuCallbacks::Back:
			app->PopState();
			return;
		}

		// TODO: scrollbar control class
		// TODO: scroll up if widget is out of bounds
		if (scrollbar.isEnabled)
//		if (menu->focus == NULL && scrollbar.isEnabled)
		{
			if (event.type == sf::Event::EventType::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Button::Left)
				{
					if (mouseIsOver(scrollbar.scrollThumb))
					{
						scrollbar.dragOffset = scrollbar.scrollThumb.getPosition() - sf::Vector2f(sf::Mouse::getPosition(*app->window));
						scrollbar.draggingThumb = true;
//						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOLD);

						originalMenuPosition = menu->getAbsolutePosition();
						originalThumbPosition = scrollbar.scrollThumb.getPosition();
					}
				}
			}
			else if (event.type == sf::Event::EventType::MouseButtonReleased)
			{
				if (scrollbar.draggingThumb)
				{
					scrollbar.draggingThumb = false;

					/*
					if (mouseIsOver(scrollbar.scrollThumb))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
					*/
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

					if (scrollerBottomPosition > scrollerMaxPosition)
						menuMove(sf::Vector2f(0, -scrollbar.scrollJump));
				}
				else if (event.mouseWheel.delta > 0) // scroll up, or move apps down
				{
					scrollbar.jumpUp();

					if (scrollerTopPosition < scrollerMinPosition)
						menuMove(sf::Vector2f(0, scrollbar.scrollJump));
				}

				testScrollBounds();
			}
			else if (event.type == sf::Event::EventType::KeyPressed && !menu->hasFocusedWidgetInside())
			{
				if (event.key.code == sf::Keyboard::Key::Down)
				{
					scrollbar.stepDown();

					if (scrollerBottomPosition > scrollerMaxPosition)
						menuMove(sf::Vector2f(0, -scrollbar.scrollStep)); // static cast to avoid pixel-imperfect placement of text
				}
				else if (event.key.code == sf::Keyboard::Key::Up)
				{
					scrollbar.stepUp();

					if (scrollerTopPosition < scrollerMinPosition)
						menuMove(sf::Vector2f(0, scrollbar.scrollStep)); // static cast to avoid pixel-imperfect placement of text
				}
				else if (event.key.code == sf::Keyboard::Key::Home)
				{
					scrollbar.moveToTop();
					menu->setPosition(sf::Vector2f(10, 10));
				}
				else if (event.key.code == sf::Keyboard::Key::End)
				{
					scrollbar.moveToBottom();
					menu->setPosition(sf::Vector2f(10, -menu->getSize().y + app->window->getSize().y - 10));
				}

				testScrollBounds();
			}
			else if (event.type == sf::Event::EventType::MouseMoved)
			{
				if (scrollbar.draggingThumb)
				{
					scrollbar.scrollThumb.setPosition(sf::Vector2f(scrollbar.scrollThumb.getPosition().x, sf::Mouse::getPosition(*app->window).y + scrollbar.dragOffset.y));
					if (!scrollbar.boundsCheck()) // scroll from the last legitimate location
						scrollbar.dragOffset = scrollbar.scrollThumb.getPosition() - sf::Vector2f(sf::Mouse::getPosition(*app->window));

					menu->setPosition(sf::Vector2f(menu->getAbsolutePosition().x, originalMenuPosition.y - ((scrollbar.scrollThumb.getPosition().y - originalThumbPosition.y) * scrollbar.scrollStep)));

//					FIXME: error zone for scrollbar (settingsstate)
//					if (sf::Mouse::getPosition(*app->window).x > scrollbar.scrollTrack.getPosition().x + 150 || 
//						sf::Mouse::getPosition(*app->window).x < scrollbar.scrollTrack.getPosition().x - 150) // error zone
//						scrollbar.scrollThumb.setPosition(scrollbar.originalPosition);
				}
				else
				{
					// TODO: find a way to not change scrollbar color every update

					/*
					if (mouseIsOver(scrollbar.scrollThumb))
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB_HOVER);
					else
						scrollbar.scrollThumb.setFillColor(GBL::theme.palatte.SCROLLTHUMB);
					*/
				}
			}
		}
	}
}

void NewsEditState::Update()
{
}

void NewsEditState::Draw()
{
	app->window->clear(SFUI::Theme::windowBgColor);

	app->window->draw(*menu);
	app->window->draw(scrollbar);

	app->window->display();
}

void NewsEditState::updateScrollThumbSize()
{
	const float contentHeight = menu->getSize().y + 20; // padding * 2 = 20

	scrollbar.update(contentHeight, menu->getSize().y);

	// TODO: update scroll limits separately
	updateScrollLimits();
}

void NewsEditState::updateScrollLimits()
{
	scrollerTopPosition = menu->getAbsolutePosition().y;
	scrollerBottomPosition = menu->getAbsolutePosition().y + menu->getSize().y + 10;
	scrollerMinPosition = 10; // navbar & padding
	scrollerMaxPosition = app->window->getSize().y;
}

void NewsEditState::testScrollBounds()
{
	updateScrollLimits();

	if (scrollerBottomPosition < scrollerMaxPosition)
	{
		menu->setPosition(sf::Vector2f(10, -menu->getSize().y + app->window->getSize().y - 10));
		updateScrollLimits();
	}
	else if (scrollerTopPosition > scrollerMinPosition)
	{
		menu->setPosition(sf::Vector2f(10, 10));
		updateScrollLimits();
	}
}

void NewsEditState::createMenu(SFUI::Menu& menu)
{
	menu.setPosition(sf::Vector2f(10, 10));

	menu.add(titleBox = new SFUI::InputBox);
	titleBox->setText(title);
	menu.add(contentBox = new MultilineInputBox(500, 25));
	contentBox->setText(content);

	SFUI::HorizontalBoxLayout* hbox = menu.addHorizontalBoxLayout();
	hbox->addButton("Upload", MenuCallbacks::Upload);
	hbox->addButton("Back", MenuCallbacks::Back);

	updateScrollThumbSize();
}

//  FIXME: app names are only properly loaded the first time this is called
void NewsEditState::loadNewsContent(const std::string& articleTitle)
{
	std::cout << "populating applist" << std::endl;

	title = "";
	content = "";

	downloadNewsFiles();

	std::ifstream readIndex(GBL::DIR::installDir + "news.txt", std::ios::in | std::ios::binary);

	if (!readIndex.is_open())
	{
		std::cerr << "failed to open news.txt" << std::endl;
		abort();
	}

	std::string line; // each line of index.dat
	bool nextLineIsNewLine = true; // starts as true because the first line is a new one
	bool hasTitle = false;
	unsigned int loopi = 0;
	while (std::getline(readIndex, line))
	{
		loopi++;

		if (nextLineIsNewLine)
		{
			if (line == articleTitle)
			{
				std::cout << "got title" << std::endl;
				newsStartPositionInFile = loopi;
				hasTitle = true;
				title = line;
			}

			nextLineIsNewLine = false;
		}
		else
			if (line == "-----------------------------")
				if (hasTitle)
				{
					std::cout << "last line of news, ending loop" << std::endl;
					break;
				}
				else
					nextLineIsNewLine = true;
			else if (hasTitle)
				content += line + "\n";
	}

	readIndex.close();
	
	// erase the last newline
	content.erase(content.length() - 1);

	std::cout << "loaded news content" << std::endl;
}

void NewsEditState::downloadNewsFiles()
{
	Download getNews;
	getNews.setInput("./" + GBL::WEB::NEWS + "/news.txt");
	getNews.setOutputDir(GBL::DIR::installDir);
	getNews.setOutputFilename("/news.txt");

	if (getNews.download() == Download::Status::Ok)
		getNews.save();
	else
		std::cerr << "failed to download news" << std::endl;
}

void NewsEditState::overwriteOldNewsContent()
{
	std::cout << "overwriting old news" << std::endl;

	std::ifstream readIndex(GBL::DIR::installDir + "news.txt", std::ios::binary);

	if (!readIndex.is_open())
	{
		std::cerr << "failed to open news.txt to read" << std::endl;
		abort();
	}
	
	std::string index((std::istreambuf_iterator<char>(readIndex)), std::istreambuf_iterator<char>());
	std::istringstream indexStream(index);

	bool onOurBlock = false;
	std::streampos startPosition = 0;
	intmax_t charactersToRemove = 0;
	for (std::string line; std::getline(indexStream, line);)
	{
		if (line == newsToEdit)
		{
			onOurBlock = true;

			// set the start position for erasing
			// this will be the end of the current line
			startPosition = indexStream.tellg();
			// so we need to go to the beginning of the current line
			startPosition -= line.length() + 1;
		}

		if (onOurBlock)
		{
			charactersToRemove += line.length() + 1;

			if (line == "-----------------------------")
			{
				std::cout << "end of article, breaking loop" << std::endl;
				break;
			}
		}
	}

	std::cout << "removing " << charactersToRemove << " characters" << std::endl;
	std::cout << "from: " << startPosition << " to " << charactersToRemove << std::endl;
	std::cout << "block to be removed: " << std::endl;
	std::cout << index.substr(startPosition, charactersToRemove) << std::endl;

	readIndex.close();

	std::ofstream writeIndex(GBL::DIR::installDir + "news.txt", std::ios::trunc | std::ios::binary);

	if (!writeIndex.is_open())
	{
		std::cerr << "failed to open news.txt to write" << std::endl;
		abort();
	}

	index.erase(startPosition, charactersToRemove);

	std::string newArticle;
	newArticle += titleBox->getText();
	newArticle += "\n";
	newArticle += contentBox->getText();
	newArticle += "\n";
	newArticle += "-----------------------------\n";

	index.insert(startPosition, newArticle);

	writeIndex << index;

	if (writeIndex.bad())
	{
		// TODO: write the temp news to a file so it's not lost

		std::cerr << "failed to rewrite news to file" << std::endl;
		abort();
	}

	writeIndex.close();

	std::cout << "updated news" << std::endl;
}

bool NewsEditState::mouseIsOver(sf::Shape &object)
{
	if (object.getGlobalBounds().contains(app->window->mapPixelToCoords(sf::Mouse::getPosition(*app->window))))
		return true;
	else
		return false;
}
