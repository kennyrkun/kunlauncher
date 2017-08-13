#include "AppEngine.hpp"
#include "AppListState.hpp"
#include "AppInfoState.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "Launching with " << argc << " arguments:" << std::endl;

	LaunchOptions settings;

	for (int i = 0; i < argc; i++)
	{
		std::cout << i << ": " << argv[i] << std::endl;

		if (std::string(argv[i]) == "-nolauncherupdate")
		{
			std::cout << "launcher will not check for updates" << std::endl;
			settings.updateLauncherOnStart = false;
		}

		if (std::string(argv[i]) == "-noindexupdate")
		{
			std::cout << "will not check for app updates" << std::endl;
			settings.updateItemsOnStart = false;
		}

		if (std::string(argv[i]) == "-width")
		{
			std::cout << "launcher width set to " << argv[i + 1] << std::endl;
			settings.width = std::stoi(argv[i + 1]);
		}
	}

	std::cout << std::endl;

	{
		AppEngine2* program = new AppEngine2("waiting");

		AppListState *appListState = new AppListState(program);
		program->ChangeState(appListState);

		while (program->Running())
		{
			if (program->Running())
				program->HandleEvents();

			if (program->Running())
				program->Update();

			if (program->Running())
				program->Draw();
		}

		delete appListState;
		delete program;
	}

	std::cin.get();

	return 0;
}