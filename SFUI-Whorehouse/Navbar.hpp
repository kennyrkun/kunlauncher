#ifndef NAVBAR_HPP
#define NAVBAR_HPP

#include <SFML/Graphics.hpp>

#include <map>

class AppEngine;
class PhysicalAnimator;

// TODO: extends sf::Drawable
// TODO: add progress bar to navbar
class Navbar
{
public:
	Navbar(sf::RenderWindow* window, PhysicalAnimator& animator);
	~Navbar();

	sf::RectangleShape bar;

	std::map<std::string, sf::Text> sections;

	bool select(const std::string& section);

	sf::Text& addSection(const std::string& section);
	void removeSection(const std::string& section); // TODO: get rid of this

//	void HandleEvents(const sf::RenderWindow* window, const sf::View& view, const sf::Event& event);
	void HandleEvents(const sf::Event& event);
	void Update();
	void Draw(); // TODO: render to rendertarget

	bool mouseIsOver(sf::Shape &object);
	bool mouseIsOver(sf::Text &object);

	PhysicalAnimator& animator;

private:
	sf::RenderWindow* window;

	sf::Vertex shadow[4];
	sf::RectangleShape selectedThing;

	std::string selectedSection;

	int nextXPositon = 0;
};

#endif // !NAVBAR_HPP
