#ifndef INETR_UPDATER_HPP
#define INETR_UPDATER_HPP

#include <cstdint>

#include <string>
#include <vector>
#include <sstream>

namespace inetr {
	class Updater {
	public:
		Updater(std::string remoteUpdateRoot);

		bool GetRemoteVersion(uint16_t *version);
		bool IsInstalledVersionUpToDate(bool &isUpToDate, uint16_t
			*upToDateVersion);
		bool ReceiveRemoteVersionInfo(uint16_t *version, std::stringstream
			&info);
		bool PrepareUpdateToRemoteVersion(uint16_t *version);
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

		uint16_t versionToUpdateTo[4];
		std::vector<std::string> remoteFilesToDownload;
	};
}

#endif  // !INETR_UPDATER_HPP
