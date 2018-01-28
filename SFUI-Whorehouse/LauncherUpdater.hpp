#ifndef LAUNCHER_UPDATER_HPP
#define LAUNCHER_UPDATER_HPP

#include <string>

class LauncherUpdater
{
public:
	enum Status
	{
		DownloadFailed,
		RequiredUpdate,
		UpdateAvailable,
		NoUpdateAvailable,

		Failure,
		Success
	};

	int getRemoteVersion();
	int remoteVersion;
	int getLocalVersion();
	int localVersion;

	int checkForUpdates();
	int downloadUpdate();
	int replaceOldExecutable();
	int removeOldExecutable();

	bool updateIsAvailable = false;
	bool restartAfterUpdate = false;
	
	bool requiredUpdate;
};

#endif // !LAUNCHER_UPDATER_HPP
