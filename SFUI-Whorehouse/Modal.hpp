#ifndef MODAL_HPP
#define MODAL_HPP

#include <string>
#include <vector>

#include <SFML\Graphics.hpp>

struct ModalOptions
{
	std::string title;
	std::string text;
	std::vector<std::string> settings;
	int width = 300;
	int height = 75;
};

class Modal
{
public:
	Modal(ModalOptions settings_);
	~Modal();

	int returnCode;

private:
	ModalOptions settings;
	sf::RenderWindow window;

	bool mouseIsOver(sf::Shape &object);
};

#endif
