#ifndef NAVBAR_HPP
#define NAVBAR_HPP

#include "NavbarSection.hpp"

#include <SFML\Graphics.hpp>
#include <vector>

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

#endif // !NAVBAR_HPP
