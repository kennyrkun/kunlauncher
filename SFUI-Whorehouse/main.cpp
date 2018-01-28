#include "AppEngine.hpp"
#include "InitialiseState.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "Launching with " << argc << " arguments:" << std::endl;

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
		app.Init("KunLauncher", settings);

		app.ChangeState(InitialiseState::Instance());

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
