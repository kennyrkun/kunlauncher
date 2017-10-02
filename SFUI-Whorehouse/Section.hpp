#ifndef SECTION_HPP
#define SECTION_HPP

#include <SFML\Graphics.hpp>

class Section : public sf::Drawable
{
public:
	Section(std::string text, std::string forwardSection, float xSize, float ySize, float xPos, float yPos);
	~Section();

	sf::RectangleShape cardShape;
	sf::RectangleShape followSectionButton;
	std::string forwardStateName;
	sf::Text title;
	bool usable;
	int totalHeight;

	void follow();
	void update(float xSize, float ySize, float xPos, float yPos);
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

private:
	sf::Texture followSectionTexture;

	sf::Font font;

	std::ifstream& GotoLine(std::ifstream& file, unsigned int num);
	void getSection(std::string from_string);
};

#endif // !SECTION_HPP
