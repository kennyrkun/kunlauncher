#include "AppEngine.hpp"
//#include "AppListState.hpp"
#include "InitialiseState.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "Launching with " << argc << " arguments:" << "\n";

	LaunchOptions settings;

	for (int i = 0; i < argc; i++)
	{
		std::cout << i << ": " << argv[i] << "\n";

		if (std::string(argv[i]) == "-nolauncherupdate")
		{
			std::cout << "launcher will not check for updates" << "\n";
			settings.updateLauncherOnStart = false;
		}

		if (std::string(argv[i]) == "-noindexupdate")
		{
			std::cout << "will not check for app updates" << "\n";
			settings.updateItemsOnStart = false;
		}

		if (std::string(argv[i]) == "-width")
		{
			std::cout << "launcher width set to " << argv[i + 1] << "\n";
			settings.width = std::stoi(argv[i + 1]);
		}
	}

	std::cout << "\n";

	{
		AppEngine* app = new AppEngine;
		app->Init("waiting");

//		app->ChangeState(AppListState::Instance());
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