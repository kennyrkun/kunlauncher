#include "NavbarSection.hpp"

#include "Globals.hpp"

#include <iostream>

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
