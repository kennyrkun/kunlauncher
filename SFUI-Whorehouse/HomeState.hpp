#ifndef HOME_STATE_HPP
#define HOME_STATE_HPP

#include "AppState.hpp"

#include <SFML/Graphics.hpp>
#include <SFUI/IconButton.hpp>
#include <SFUI/Scrollbar.hpp>
#include <vector>

//class Section;

class NavbarSection
{
public:
	NavbarSection(std::string str, int sectionNUm);
	~NavbarSection();

	void update();

	int sectionNum;
	sf::Text text;

private:
	std::string str;
	sf::Font font;
};

class Navbar
{
public:
	Navbar(sf::RenderWindow* window);
	~Navbar();

	sf::RectangleShape bar;

	std::vector<NavbarSection*> sections;

	void addSection(std::string text);
	void removeSection(int sectionNum);

	void HandleEvents(const sf::Event& event);
	void Update();
	void Draw();

private:
	sf::RenderWindow* window;
	sf::Font font;
};

class News
{
public:
	News(std::string title, std::string text, sf::RenderWindow* window);
	~News();

	sf::Text title;
	sf::Text text;
	sf::RectangleShape divider;

	void setPosition(const sf::Vector2f& pos);
	sf::Vector2f getPosition();
	float getLocalHeight();

	void HandleEvents(const sf::Event& event);
	void Update();
	void Draw();

private:
	std::string titlestr;
	std::string textstr;

	sf::Font font;

	sf::RenderWindow* window;
};

class HomeState : public AppState
{
public:
	void Init(AppEngine* app_);
	void Cleanup();

	void Pause();
	void Resume();

	void HandleEvents();
	void Update();
	void Draw();

	static HomeState* Instance()
	{
		return &HomeState_dontfuckwithme;
	}

protected:
	HomeState() { }

private:
	static HomeState HomeState_dontfuckwithme;
	AppEngine* app;

	std::vector<std::thread> threads;
	std::vector<SFUI::IconButton*> sections;
	std::vector<News*> newses;

	sf::Font font;

	Navbar* navbar;

	void loadNews(bool &finishedIndicator);

	sf::View* viewScroller;
	sf::View* mainView;
	SFUI::Scrollbar scrollbar;

	void updateScrollThumbSize();
	// TODO: viewable arae class
	float scrollerTopPosition;
	float scrollerBottomPosition;
	float scrollerMinPosition;
	float scrollerMaxPosition;
	void updateScrollLimits();

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Shape &object, sf::View* view);
	bool mouseIsOver(sf::Text &object);
};

#endif // !HOME_STATE_HPP