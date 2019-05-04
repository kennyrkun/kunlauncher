#ifndef LAUNCHER_UPDATER_HPP
#define LAUNCHER_UPDATER_HPP

// TODO: changed_files files with a list of files needing to be updated

// KunLaucher uses something like semantic versioning.
// MAJOR . MINOR . PATCH
// major is big new features
// minor is small, QoL features
// patch is bug fixes and such

#include "Globals.hpp"

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

	GBL::Version getRemoteVersion();
	GBL::Version remoteVersion;
	GBL::Version& getLocalVersion();
	GBL::Version localVersion;

	int checkForUpdates();
	int downloadUpdate();
	int replaceOldExecutable();
	int removeOldExecutable();

	bool updateIsAvailable = false;
	bool restartAfterUpdate = false;
	
	bool requiredUpdate;
};

#endif // !LAUNCHER_UPDATER_HPP
