#ifndef LAUNCHER_UPDATER_HPP
#define LAUNCHER_UPDATER_HPP

#include <string>

class LauncherUpdater
{
public:
	enum Status
	{
		DownloadFailed,
		UpdateAvailable,
		NoUpdateAvailable,

		Failure,
		Success
	};

	std::string getRemoteVersion();
	std::string getLocalVersion();

	int checkForUpdates();
	int downloadUpdate();
	int replaceOldExecutable();
	int removeOldExecutable();

	int createUpdateFile();
	int readUpdateFile();

	bool updateIsAvailable = false;
	bool restartAfterUpdate = false;
	std::string remoteVersion;
	std::string localVersion;

	bool requiredUpdate;
};

#endif // !LAUNCHER_UPDATER_HPP
