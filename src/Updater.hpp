#ifndef INETR_UPDATER_HPP
#define INETR_UPDATER_HPP

#include <string>
#include <vector>
#include <sstream>

namespace inetr {
	enum VersionComparisonResult { VCR_Newer, VCR_Older, VCR_Equal };

	class Updater {
	public:
		Updater(std::string remoteUpdateRoot);

		bool GetInstalledVersion(unsigned short *version);
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
		
		void VersionStrToArr(std::string &verStr, unsigned short *version);
		void VersionArrToStr(unsigned short *version, std::string &verStr,
			bool omitTrailingZeros = false);
		VersionComparisonResult CompareVersions(unsigned short *v1,
			unsigned short *v2);

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