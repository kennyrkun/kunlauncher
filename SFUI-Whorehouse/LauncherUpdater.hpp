#ifndef LAUNCHER_UPDATER_HPP
#define LAUNCHER_UPDATER_HPP

// TODO: changed_files files with a list of files needing to be updated

// KunLaucher uses something like semantic versioning.
// MAJOR . MINOR . PATCH
// major is big new features
// minor is small, QoL features
// patch is bug fixes and such

// todo: update channels
// buildtype = release
// only download updates with "release" in them

// TODO: if we are unable to recognise the version format, try to update anyway
// rename old executable to allow people to go back if it fucks itself up

#include "Globals.hpp"

class LauncherUpdater
{
public:
	enum Status
	{
		RequiredUpdate,
		UpdateAvailable,
		NoUpdateAvailable,

		DownloadFailed,
		Failure,

		Success
	};

	LauncherUpdater();

	GBL::LauncherVersion remoteVersion;
	GBL::LauncherVersion localVersion = GBL::version;

	int checkForUpdates();
	int downloadUpdate();
	int replaceOldExecutable();
	int removeOldExecutable();

	bool updateIsAvailable = false;
	bool restartAfterUpdate = false;
	
	bool requiredUpdate;

private:
	GBL::LauncherVersion getRemoteVersion();
};

#endif // !LAUNCHER_UPDATER_HPP
