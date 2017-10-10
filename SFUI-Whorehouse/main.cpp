#include "AppEngine.hpp"
//#include "AppListState.hpp"
#include "InitialiseState.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "Launching with " << argc << " arguments:" << "\n";

	AppSettings settings;

	for (int i = 0; i < argc; i++)
	{
		std::cout << i << ": " << argv[i] << "\n";

		if (std::string(argv[i]) == "-verticalsync")
		{
			if (std::string(argv[i + 1]) == "1")
				settings.verticalSync = true;
			else if (std::string(argv[i + 1]) == "0")
				settings.verticalSync = false;
		}

		if (std::string(argv[i]) == "-nolauncherupdate")
		{
			std::cout << "launcher will not check for updates" << "\n";
			settings.updateLauncherOnStart = false;
		}

		if (std::string(argv[i]) == "-noindexupdate")
		{
			std::cout << "will not check for app updates" << "\n";
			settings.updateItemIndexOnStart = false;
		}

		if (std::string(argv[i]) == "-width")
		{
			std::cout << "launcher width set to " << argv[i + 1] << "\n";
			settings.width = std::stoi(argv[i + 1]);
		}

		if (std::string(argv[i]) == "-height")
		{
			std::cout << "launcher height set to " << argv[i + 1] << "\n";
			settings.height = std::stoi(argv[i + 1]);
		}
	}

	std::cout << "\n";

	{
		AppEngine* app = new AppEngine;
		app->Init("KunLauncher", settings);

		app->ChangeState(InitialiseState::Instance());

		while (app->Running())
		{
			app->HandleEvents();
			app->Update();
			app->Draw();
		}

		app->Cleanup();
	}

	std::cin.get();

	return 0;
}