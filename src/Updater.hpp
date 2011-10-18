#ifndef INETR_UPDATER_HPP
#define INETR_UPDATER_HPP

#include <string>
#include <vector>
#include <sstream>

namespace inetr {
	class Updater {
	public:
		Updater(std::string remoteUpdateRoot);

		bool GetRemoteVersion(unsigned short *version);
		bool IsInstalledVersionUpToDate(bool &isUpToDate, unsigned short
			*upToDateVersion);
		bool ReceiveRemoteVersionInfo(unsigned short *version, std::stringstream
			&info);
		bool PrepareUpdateToRemoteVersion(unsigned short *version);
		bool LaunchPreparedUpdateProcess();
		bool PerformPreparedUpdate();
		
		bool WriteUpdateInformationToSharedMemory();
		bool FetchUpdateInformationFromSharedMemory();
		void FreeUpdateInformationSharedMemory();

		std::vector<std::string> OptionalFiles;
	private:
		std::string remoteUpdateRoot;

		void *versionToUpdateToMapping;
		void *remoteFilesToDownloadSizeMapping;
		void *remoteFilesToDownloadMapping;

		unsigned short versionToUpdateTo[4];
		std::vector<std::string> remoteFilesToDownload;
	};
}

#endif // !INETR_UPDATER_HPP