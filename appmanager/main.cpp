#include "../SFUI-Whorehouse/AppEngine.hpp"
#include "../SFUI-Whorehouse/SettingsParser.hpp"
#include "InitialiseState.hpp"

#include <SFML/Graphics.hpp>
#include <SFUI/SFUI.hpp>
#include <SFUI/Theme.hpp>

// TODO: something that checks for issues in the issue reporter
// TODO: use launchoptions struct in appengine
// TOOD: camelcase all enums

int main(int argc, char *argv[])
{
	std::cout << "Launching with " << argc << " arguments: " << std::endl;
	
	for (size_t i = 0; i < argc; i++)
		std::cout << i << ": " << argv[i] << std::endl;

	SFUI::Theme::loadFont("../SFUI-Whorehouse/bin/resources/fonts/Tahoma.ttf");
	SFUI::Theme::loadTexture("../SFUI-Whorehouse/bin/resources/textures/interface_square.png");
	SFUI::Theme::textCharacterSize = 11;
	SFUI::Theme::click.textColor = SFUI::Theme::hexToRgb("#191B18");
	SFUI::Theme::click.textColorHover = SFUI::Theme::hexToRgb("#191B18");
	SFUI::Theme::click.textColorFocus = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::input.textColor = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::input.textColorHover = SFUI::Theme::hexToRgb("#CC7A00");
	SFUI::Theme::input.textColorFocus = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::windowBgColor = sf::Color(40, 40, 40);
	SFUI::Theme::PADDING = 2.f;

	AppSettings settings;

	/* TODO: don't get overriden by config
	for (int i = 0; i < argc; i++)
	{
	std::cout << i << ": " << argv[i] << std::endl;

	if (std::string(argv[i]) == "-noverticalsync")
	{
	std::cout << "vertical sync disabled" << std::endl;
	settings.verticalSync = false;
	}

	if (std::string(argv[i]) == "-nolauncherupdate")
	{
	std::cout << "launcher will not check for updates" << std::endl;
	settings.updateLauncherOnStart = false;
	}

	if (std::string(argv[i]) == "-noindexupdate")
	{
	std::cout << "will not check for app updates" << std::endl;
	settings.checkForNewItemsOnStart = false;
	}

	if (std::string(argv[i]) == "-width")
	{
	std::cout << "launcher width set to " << argv[i + 1] << std::endl;
	settings.width = std::stoi(argv[i + 1]);
	}

	if (std::string(argv[i]) == "-height")
	{
	std::cout << "launcher height set to " << argv[i + 1] << std::endl;
	settings.height = std::stoi(argv[i + 1]);
	}
	}
	*/

	std::cout << std::endl;

	{
		AppEngine app;
		app.Init("App Manager", settings);

		app.PushState(new InitialiseState);

		while (app.Running())
		{
			app.HandleEvents();
			app.Update();
			app.Draw();
		}

		app.Cleanup();
	}

	return 0;
}

/*
int mai2n()
{
	enum Callback
	{
		C_TEXT,
		C_COLOR,
		C_ROTATION,
		C_BOLD,
		C_UNDERLINED,
		C_SCALE,
		C_NEW_BUTTON,
		C_VSLIDER,
		C_THEME_TEXTURE,
		C_TERMINATE,
		C_QUIT
	};

	sf::RenderWindow window(sf::VideoMode(640, 480), "KunLauncher Uploader", sf::Style::Titlebar | sf::Style::Close);

	SFUI::Theme::loadFont("resources/interface/tahoma.ttf");
	SFUI::Theme::loadTexture("resources/interface/texture_square.png");
	SFUI::Theme::textCharacterSize = 11;
	SFUI::Theme::click.textColor = SFUI::Theme::hexToRgb("#191B18");
	SFUI::Theme::click.textColorHover = SFUI::Theme::hexToRgb("#191B18");
	SFUI::Theme::click.textColorFocus = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::input.textColor = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::input.textColorHover = SFUI::Theme::hexToRgb("#CC7A00");
	SFUI::Theme::input.textColorFocus = SFUI::Theme::hexToRgb("#000000");
	SFUI::Theme::windowBgColor = SFUI::Theme::hexToRgb("#dddbde");
	SFUI::Theme::PADDING = 2.f;

	// create a menu 
	SFUI::Menu menu(window);
	// set the origin of the menu to 10, 10
	menu.setPosition(sf::Vector2f(10, 10));

	SFUI::HorizontalBoxLayout* hbox = menu.addHorizontalBoxLayout();
	SFUI::FormLayout* form = hbox->addFormLayout();
	SFUI::VerticalBoxLayout* vbox = hbox->addVerticalBoxLayout();
	SFUI::HorizontalBoxLayout* hbox2 = vbox->addHorizontalBoxLayout();
	SFUI::HorizontalBoxLayout* hbox3 = vbox->addHorizontalBoxLayout();

	// Textbox
	SFUI::InputBox* textbox = new SFUI::InputBox();
	textbox->setText("Hello, World!");
	form->addRow("Text", textbox, C_TEXT);

	// Slider for rotation
	SFUI::Slider* sliderRotation = new SFUI::Slider();
	sliderRotation->setQuantum(1);
	form->addRow("Rotation", sliderRotation, C_ROTATION);

	// Progress bar
	SFUI::ProgressBar* pbar0 = new SFUI::ProgressBar();
	form->add(pbar0);

	// Slider for scale
	SFUI::Slider* sliderScale = new SFUI::Slider();
	form->addRow("Scale", sliderScale, C_SCALE);

	// OptionsBox for color
	SFUI::OptionsBox<sf::Color>* opt = new SFUI::OptionsBox<sf::Color>();
	opt->addItem("Red", sf::Color::Red);
	opt->addItem("Blue", sf::Color::Blue);
	opt->addItem("Green", sf::Color::Green);
	opt->addItem("Yellow", sf::Color::Yellow);
	opt->addItem("White", sf::Color::White);
	form->addRow("Color", opt, C_COLOR);

	// Checbkox
	SFUI::CheckBox* checkboxBold = new SFUI::CheckBox();
	form->addRow("Bold text", checkboxBold, C_BOLD);

	SFUI::CheckBox* checkboxUnderlined = new SFUI::CheckBox();
	form->addRow("Underlined text", checkboxUnderlined, C_UNDERLINED);

	// Custom button
	sf::Texture spriteButton_texture;
	spriteButton_texture.loadFromFile("resources/interface/themed-button.png");

	SFUI::SpriteButton* customButton = new SFUI::SpriteButton(spriteButton_texture, "Custom Button");

	customButton->setTextSize(18);
	form->add(customButton);

	// Textbox
	SFUI::InputBox* textureOverrideBox = new SFUI::InputBox();
	textureOverrideBox->setText("resources/interface/texture_square.png");
	form->addRow("Theme Texture", textureOverrideBox, C_THEME_TEXTURE);

	vbox->addLabel("This panel is on the left");

	// Textbox
	SFUI::InputBox* textbox2 = new SFUI::InputBox(100);
	textbox2->setText("Button name");
	hbox2->add(textbox2, C_NEW_BUTTON);
	hbox2->addButton("Create button", C_NEW_BUTTON);

	// Small progress bar
	hbox3->addLabel("Small progress bar");
	SFUI::ProgressBar* pbar = new SFUI::ProgressBar(40);
	hbox3->add(pbar);

	SFUI::Slider* vslider = new SFUI::Slider(100, SFUI::Slider::Type::Vertical);
	hbox->add(vslider, C_VSLIDER);

	SFUI::CheckBox* terminateProgram = new SFUI::CheckBox();
	form->addRow("Terminate Program?", terminateProgram, C_TERMINATE);

	menu.addButton("Quit", C_QUIT);

	sf::Texture texture;
	texture.loadFromFile("resources/textures/sfml.png");

	sf::Sprite sprite(texture);
	sprite.setOrigin(texture.getSize().x, texture.getSize().y);
	sprite.setPosition(window.getSize().x - 10, window.getSize().y - 10);

	sf::Text text(textbox->getText(), SFUI::Theme::getFont());
	text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
	text.setPosition(window.getSize().x / 2, 400);

	// Start the game loop
	while (window.isOpen())
	{
		// Process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			int id = menu.onEvent(event);
			switch (id)
			{
			case C_TEXT:
				text.setString(textbox->getText());
				text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2);
				break;
			case C_COLOR:
				text.setFillColor(opt->getSelectedValue());
				break;
			case C_ROTATION:
				text.setRotation(sliderRotation->getValue() * 360 / 100.f);
				pbar0->setValue(sliderRotation->getValue());
				break;
			case C_UNDERLINED:
			{
				int style = text.getStyle();
				if (checkboxUnderlined->isChecked())
					style |= sf::Text::Underlined;
				else
					style &= ~sf::Text::Underlined;
				text.setStyle(style);
				break;
			}
			case C_BOLD:
			{
				int style = text.getStyle();
				if (checkboxBold->isChecked())
					style |= sf::Text::Bold;
				else
					style &= ~sf::Text::Bold;
				text.setStyle(style);
				break;
			}
			case C_SCALE:
			{
				float scale = 1 + sliderScale->getValue() * 2 / 100.f;
				text.setScale(scale, scale);
				break;
			}
			case C_QUIT:
				window.close();
				break;
			case C_VSLIDER:
				pbar->setValue(vslider->getValue());
				break;
			case C_NEW_BUTTON:
				vbox->addButton(textbox2->getText());
				break;
			case C_THEME_TEXTURE:
				SFUI::Theme::loadTexture(textureOverrideBox->getText());
//			case C_TERMINATE:
//				doTerminateProgram = !doTerminateProgram;
			}

			// Close window : exit
			if (event.type == sf::Event::Closed)
				window.close();
		}

		window.clear(SFUI::Theme::windowBgColor);

		window.draw(menu);
		window.draw(text);
		window.draw(sprite);

		window.display();
	}

	return EXIT_SUCCESS;
}
*/
