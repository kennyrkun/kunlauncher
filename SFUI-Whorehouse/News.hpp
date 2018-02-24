#ifndef NEWS_HPP
#define NEWS_HPP

#include <string>
#include <SFML\Graphics.hpp>

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

#endif // !NEWS_HPP
