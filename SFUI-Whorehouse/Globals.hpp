#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include "Theme.hpp"
#include "ThreadManager.hpp"

#include <SFUI/Theme.hpp>

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Font.hpp>

#include <string>

// TODO: improve Globals system
// TODO: Windows wrapper
//		 - beep
//		 - flash window
//		 - shellex

#ifndef _DEBUG
	#define derr(...)
	#define dout(...)
#else
	#define derr(...) std::cerr << "\a[DEBUG] " << __VA_ARGS__ << std::endl;
	#define dout(...) std::cout << "[DEBUG] " << __VA_ARGS__ << std::endl;
#endif

namespace GBL
{
	extern Theme theme;
	extern ThreadManager threadManager;

	const std::string appName = "KunLauncher";

	const float release = 17;

	namespace DIR
	{
		const std::string installDir = "./bin/";
		const std::string apps		 = installDir + "apps/";
		const std::string cache		 = installDir + "cache/";
		const std::string appcache	 = installDir + "appcache/";
		const std::string stats		 = installDir + "stats/";
		const std::string resources  = installDir + "resources/";
		const std::string textures	 = resources + "textures/";
		const std::string fonts		 = resources + "fonts/";
		const std::string themes	 = resources + "themes/";
	}

	namespace WEB
	{
		const std::string HOSTNAME	= "kunlauncher.000webhostapp.com/";
		const std::string APPS		= "apps/";
		const std::string NEWS		= "news/";
		const std::string VERSIONS	= "version/";
		const std::string ISSUES    = "issuereports/";

		namespace LATEST
		{
			const std::string DIR			= VERSIONS + "latest/";
			const std::string RESOURCES		= DIR + "resources/";
			const std::string RESOURCE_LIST = RESOURCES + "resources.dat";
			const std::string EXECUTABLE	= DIR + "latest.noexe";
			const std::string CHANGES		= DIR + "change.log";
		}
	}

	namespace CONFIG
	{
		const std::string config = DIR::installDir + "kunlauncher.conf";

		const std::string updateOnStart = "updateOnStart";
		const std::string logDownloads = "logDownloads";
		const std::string SFUIDebug = "SFUIDebug";
		const std::string allowStatTracking = "allowStatTracking";
		const std::string useAnimations = "useAnimations";
		const std::string animationScale = "animationScale";
		const std::string selectedTheme = "selectedTheme";

		namespace News
		{
			const std::string newsEnabled = "news.enabled";
		}
		
		namespace Window
		{
			const std::string verticalSync = "window.verticalSync";
			const std::string width = "window.width";
			const std::string height = "window.height";
		}

		namespace Apps
		{ 
			const std::string updateStoreOnStart = "apps.updateStoreOnStart";
			const std::string autoUpdate = "apps.autoUpdate";
			const std::string checkForUpdates = "apps.checkForUpdates";
		}
	}

	namespace MESSAGE_BOXES
	{
		void cantOpenNotWindows();
	}
}

#endif // !GLOBALS_HPP
