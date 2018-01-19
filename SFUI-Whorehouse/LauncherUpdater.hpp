#ifndef LAUNCHER_UPDATER_HPP
#define LAUNCHER_UPDATER_HPP

#include <string>

struct Version
{
	std::string major_s;
	std::string minor_s;
	std::string patch_s;

	int major;
	int minor;
	int patch;
};

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
