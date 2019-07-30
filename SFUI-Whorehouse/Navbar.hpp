#ifndef NAVBAR_HPP
#define NAVBAR_HPP

#include "AppEngine.hpp"

#include <SFML\Graphics.hpp>
#include <vector>
#include <map>

// TODO: extends sf::Drawable
// TODO: add progress bar to navbar
class Navbar
{
public:
	Navbar(sf::RenderWindow* window);
	~Navbar();

	sf::RectangleShape bar;

	// TODO: simplify the navbar
//	std::map<std::string, sf::Text> sections;
	std::vector<sf::Text> sections;

	sf::Text& addSection(std::string text);
	sf::Text& getSection(std::string sectionName);
	void removeSection(int sectionNum); // TODO: get rid of this
	void removeSection(std::string sectionName); // TODO: get rid of this

//	void HandleEvents(const sf::RenderWindow* window, const sf::View& view, const sf::Event& event);
	void HandleEvents(const sf::Event& event);
	void Update();
	void Draw(); // TODO: render to rendertarget

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Text &object);

private:
	sf::RenderWindow* window;

	sf::Vertex shadow[4];
};

#endif // !NAVBAR_HPP
