#include "Updater.hpp"

#include <algorithm>
#include <sstream>
#include <fstream>

#include <Windows.h>

#include "../resource/resource.h"
#include "StringUtil.hpp"
#include "HTTP.hpp"
#include "CryptUtil.hpp"
#include "INETRException.hpp"
#include "MUtil.hpp"

using namespace std;

namespace inetr {
	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodepage;
	};

	Updater::Updater(string remoteUpdateRoot) {
		this->remoteUpdateRoot = remoteUpdateRoot;
	}

	bool Updater::GetInstalledVersion(unsigned short *version) {
		HMODULE hMod = GetModuleHandle(nullptr);

		HRSRC hVersion = FindResource(hMod, MAKEINTRESOURCE(VS_VERSION_INFO),
			RT_VERSION);
		if (!hVersion)
			return false;

		HGLOBAL hGlobal = LoadResource(hMod, hVersion);
		if (!hGlobal)
			return false;

		LPVOID versionInfo = LockResource(hGlobal);
		if (!versionInfo) {
			FreeResource(hGlobal);
			return false;
		}

		LANGANDCODEPAGE *lpTranslate;
		UINT cbTranslate;

		BOOL retVal = VerQueryValue(versionInfo, "\\VarFileInfo\\Translation",
			(LPVOID*)&lpTranslate, &cbTranslate);

		if (!retVal || cbTranslate != 4) {
			UnlockResource(hGlobal);
			FreeResource(hGlobal);
			return false;
		}

		char versionEntry[128];
		sprintf_s(versionEntry, sizeof(versionEntry),
			"\\StringFileInfo\\%04x%04x\\FileVersion",
			lpTranslate[0].wLanguage, lpTranslate[0].wCodepage);

		char* lpVerStr = nullptr;
		UINT cbVerStr = 0;

		retVal = VerQueryValue(versionInfo, versionEntry, (LPVOID*)&lpVerStr,
			&cbVerStr);

		UnlockResource(hGlobal);
		FreeResource(hGlobal);

		if (!retVal && cbVerStr != 8)
			return false;

		string verStr(lpVerStr);
		VersionStrToArr(verStr, version);

		return true;
	}

	bool Updater::GetRemoteVersion(unsigned short *version) {
		stringstream versionFileStream;
		try {
			HTTP::Get(remoteUpdateRoot + "/version", &versionFileStream);
		} catch (INETRException) {
			return false;
		}

		string versionStr;
		versionFileStream >> versionStr;
		VersionStrToArr(versionStr, version);
		return (versionStr != "");
	}

	bool Updater::IsInstalledVersionUpToDate(bool &isUpToDate, unsigned short
		*upToDateVersion) {
		unsigned short installedVersion[4], remoteVersion[4];
		if (!GetInstalledVersion(installedVersion))
			return false;
		if (!GetRemoteVersion(remoteVersion))
			return false;
		if (isUpToDate = (CompareVersions(remoteVersion, installedVersion) !=
			VCR_Newer)) {

			upToDateVersion = nullptr;
		} else {
			memcpy(upToDateVersion, remoteVersion, sizeof(remoteVersion));
		}
		return true;
	}

	bool Updater::ReceiveRemoteVersionInfo(unsigned short *version,
		stringstream &info) {

		string versionStr;
		VersionArrToStr(version, versionStr, true);

		try {
			HTTP::Get(remoteUpdateRoot + "/" + versionStr + "/changelog",
				&info);
		} catch (INETRException) {
			return false;
		}

		return true;
	}

	bool Updater::PrepareUpdateToRemoteVersion(unsigned short *version) {
		memset(versionToUpdateTo, 0, sizeof(versionToUpdateTo));
		remoteFilesToDownload.empty();

		string versionStr;
		VersionArrToStr(version, versionStr, true);

		stringstream remoteChecksumsStream;
		try {
			HTTP::Get(remoteUpdateRoot + "/" + versionStr + "/" + INETR_ARCH +
				"/checksums", &remoteChecksumsStream);
		} catch (INETRException) {
			return false;
		}

		map<string, string> remoteFileChecksums;
		while (remoteChecksumsStream.good()) {
			string filePathAndChecksum;
			remoteChecksumsStream >> filePathAndChecksum;
			if (filePathAndChecksum == "")
				continue;
			vector<string> filePathAndChecksumSplit = StringUtil::Explode(
				filePathAndChecksum, ":");
			remoteFileChecksums.insert(pair<string, string>(
				filePathAndChecksumSplit[0], filePathAndChecksumSplit[1]));
		}

		for (map<string, string>::iterator it = remoteFileChecksums.begin();
			it != remoteFileChecksums.end(); ++it) {

			ifstream localFileStream(it->first);
			if (localFileStream.is_open()) {
				localFileStream.close();
				string md5Hash;
				try {
					md5Hash = CryptUtil::FileMD5Hash(it->first);
				} catch (INETRException) {
					return false;
				}
				if (md5Hash != "" && md5Hash == it->second)
					continue;
			} else if (find(OptionalFiles.begin(), OptionalFiles.end(),
				it->first) != OptionalFiles.end()) {

				continue;
			}

			remoteFilesToDownload.push_back(it->first);
		}

		if (remoteFilesToDownload.empty())
			return false;

		memcpy(versionToUpdateTo, version, sizeof(versionToUpdateTo));

		return true;
	}

	bool Updater::PerformPreparedUpdate() {
		if (reinterpret_cast<unsigned long long>(versionToUpdateTo) == 0L ||
			remoteFilesToDownload.empty())
			return false;

		for (vector<string>::iterator it = remoteFilesToDownload.begin();
			it != remoteFilesToDownload.end(); ++it) {

			string remoteFilename = *it;
			StringUtil::SearchAndReplace(remoteFilename, "\\", "/");
			string localFilename = *it;
			StringUtil::SearchAndReplace(localFilename, "/", "\\");

			string versionStr;
			VersionArrToStr(versionToUpdateTo, versionStr, true);

			string remoteURL = remoteUpdateRoot + "/" + versionStr + "/" +
				INETR_ARCH + "/" + remoteFilename;

			string localTmpFilename = localFilename + ".updatetmp";
			MoveFile(localFilename.c_str(), localTmpFilename.c_str());

			stringstream remoteFileStream;
			try {
				HTTP::Get(remoteURL, &remoteFileStream);
			} catch (INETRException) {
				return false;
			}

			ofstream localFileStream;
			localFileStream.open(localFilename, ios::out | ios::binary);

			localFileStream << remoteFileStream.rdbuf();

			localFileStream.close();

			DeleteFile(localTmpFilename.c_str());
		}

		return true;
	}

	void Updater::VersionStrToArr(string &verStr, unsigned short *version) {
		vector<string> verDigitStrs = StringUtil::Explode(verStr, ".");

		for (size_t i = 0; i < 4; ++i) {
			if (verDigitStrs.size() > i)
				version[i] = atoi(verDigitStrs[i].c_str());
			else
				version[i] = 0;
		}
	}

	void Updater::VersionArrToStr(unsigned short *version, string &verStr,
		bool omitTrailingZeros /* = false */) {

		for (size_t i = 4; i > 0; --i) {
			if (!(omitTrailingZeros && version[i - 1] == 0 && verStr == "")) {
				stringstream ssVerNbr;
				ssVerNbr << version[i - 1];
				if (verStr != "")
					verStr = "." + verStr;
				verStr = ssVerNbr.str() + verStr;
			}
		}
	}

	VersionComparisonResult Updater::CompareVersions(unsigned short *v1,
		unsigned short *v2) {

		for (size_t i = 0; i < 4; ++i) {
			if (v1[i] > v2[i])
				return VCR_Newer;
			else if (v1[i] < v2[i])
				return VCR_Older;
		}

		return VCR_Equal;
	}
}