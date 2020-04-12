#ifndef NEWS_HPP
#define NEWS_HPP

#include <SFML/Graphics.hpp>

#include <string>

// TODO: links to the news and ratings and stuff
// TODO: much better news system
// TODO: decrease news parse time

// give each news piece an ID

class News : public sf::Drawable
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

	void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
	// TODO: wrap title
	std::string titlestr; // TODO: what's this for?
	std::string textstr; // TODO: what's this for?

	std::string display_TextString;
	const std::string original_TextString;
};

#endif // !NEWS_HPP
