#include "AppEngine.hpp"
#include "InitialiseState.hpp"

#include <iostream>

// FIXME: cog icon not loading after download
// TODO: optimise file includes
// TODO: only redraw when necessary

int main(int argc, char *argv[])
{
	std::cout << "Launching with " << argc << " arguments:" << std::endl;

	AppSettings settings;

	// settings.load(argv);

	// TODO: error handling for potentially invalid arguments
	for (int i = 0; i < argc; i++)
	{
		std::cout << i << ": " << argv[i] << std::endl;

		std::string currentArg(argv[i]);

		if (currentArg == "-verticalSync")
		{
			settings.window.verticalSync = std::stoi(argv[i + 1]);

			std::cout << "verticalSync ";
			if (settings.window.verticalSync)
				std::cout << "enabled" << std::endl;
			else
				std::cout << "disabled" << std::endl;
		}

		if (currentArg == "-noUpdateOnStart")
		{
			settings.updateOnStart = std::stoi(argv[i + 1]);

			std::cout << "updateOnStart ";
			if (settings.updateOnStart)
				std::cout << "enabled" << std::endl;
			else
				std::cout << "disabled" << std::endl;
		}

		if (currentArg == "-apps.updateStoreOnStart")
		{
			settings.apps.updateStoreOnStart = std::stoi(argv[i + 1]);

			std::cout << "apps.updateStoreOnStart  ";
			if (settings.apps.updateStoreOnStart)
				std::cout << "enabled" << std::endl;
			else
				std::cout << "disabled" << std::endl;
		}

		if (currentArg == "-apps.autoUpdate")
		{
			settings.apps.autoUpdate = std::stoi(argv[i + 1]);

			std::cout << "apps.autoUpdate  ";
			if (settings.apps.autoUpdate)
				std::cout << "enabled" << std::endl;
			else
				std::cout << "disabled" << std::endl;
		}

		if (currentArg == "-window.width")
		{
			std::cout << "window.width set to " << argv[i + 1] << std::endl;
			settings.window.width = std::stoi(argv[i + 1]);
		}

		if (currentArg == "-window.height")
		{
			std::cout << "window.height set to " << argv[i + 1] << std::endl;
			settings.window.height = std::stoi(argv[i + 1]);
		}

		/*
		if (currentArg == "-state")
		{
			std::cout << "not opening into homestate" << std::endl;
			// TODO: boot into state
			// but don't skip initialisestate
		}
		*/
	}

	std::cout << std::endl;

	{
		AppEngine app;
		app.Init("KunLauncher", settings);

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
