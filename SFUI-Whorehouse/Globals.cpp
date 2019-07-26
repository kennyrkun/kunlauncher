#include "Globals.hpp"
#include "MessageBox.hpp"

namespace GBL
{
	Theme theme;
	ThreadManager threadManager;

	namespace COLOR
	{
		sf::Color BACKGROUND = sf::Color(50, 50, 50);
		sf::Color TEXT = sf::Color(255, 255, 255);

		sf::Color PRIMARY = sf::Color(100, 100, 100);
		sf::Color SECONDARY = sf::Color(150, 150, 150);
		sf::Color TERTIARY = sf::Color(50, 50, 50);

		namespace SCROLLBAR
		{
			sf::Color SCROLLBAR = sf::Color(80, 80, 80);
			sf::Color SCROLLTHUMB = sf::Color(110, 110, 110);
			sf::Color SCROLLTHUMB_HOVER = sf::Color(158, 158, 158);
			sf::Color SCROLLTHUMB_HOLD = sf::Color(239, 235, 239);
		}

		namespace APP
		{
			sf::Color CARD = sf::Color(100, 100, 100);
			sf::Color CARD2 = sf::Color(255, 255, 255);
			sf::Color IMAGE = sf::Color(255, 255, 255);
			sf::Color ICON = sf::Color(255, 255, 255);
			sf::Color ICON_HOVER = sf::Color(255, 255, 255);
			sf::Color ICON_PRESS = sf::Color(255, 255, 255);
			sf::Color ICON_FAIL = sf::Color(255, 255, 255);
			sf::Color ICON_FAIL_HOVER = sf::Color(255, 255, 255);
			sf::Color ICON_FAIL_PRESS = sf::Color(255, 255, 255);
			sf::Color ICON_WARN = sf::Color(255, 200, 0);
			sf::Color ICON_WARN_HOVER = sf::Color(255, 255, 255);
			sf::Color ICON_WARN_PRESS = sf::Color(255, 255, 255);
		}

		namespace LINK
		{
			sf::Color CARD = sf::Color(100, 100, 100);
			sf::Color CARD2 = sf::Color(255, 255, 255);
			sf::Color FOLLOW = sf::Color(255, 255, 255);
			sf::Color TEXT = sf::Color(0, 170, 232);
			sf::Color ICON = sf::Color(255, 255, 255);
			sf::Color ICON_HOVER = sf::Color(255, 255, 255);
		}
	}
}

void GBL::MESSAGE_BOXES::cantOpenNotWindows()
{
	std::cerr << "KunLauncher doesn't know how to open things on this operating system." << std::endl;

	MessageBox::Options modOptions;
	modOptions.title = "Unsupported Platform";
	modOptions.text = "KunLauncher has only been taught how to operate fully on the following platforms:\n\n- Windows";
	modOptions.settings = { "Sad", "Very Unfortunate" };

	MessageBox platformAlert(modOptions);
	platformAlert.runBlocking();
}
