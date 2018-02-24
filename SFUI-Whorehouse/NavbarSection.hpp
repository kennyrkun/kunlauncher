#ifndef NAVBAR_SECTION_HPP
#define NAVBAR_SECTION_HPP

#include <SFML\Graphics.hpp>
#include <string>

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

#endif // !NAVBAR_SECTION_HPP
