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

	bool Updater::LaunchPreparedUpdateProcess() {
		if (reinterpret_cast<unsigned long long>(versionToUpdateTo) == 0L ||
			remoteFilesToDownload.empty())
			return false;

		if (!WriteUpdateInformationToSharedMemory())
			return false;

		char modulePath[MAX_PATH];
		GetModuleFileName(GetModuleHandle(nullptr), modulePath,
			sizeof(modulePath));

		string sModulePath(modulePath);
		size_t lastDelim = sModulePath.find_last_of("\\");
		if (lastDelim == string::npos)
			return false;

		SHELLEXECUTEINFO shExInfo;
		ZeroMemory(&shExInfo, sizeof(shExInfo));
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = nullptr;
		shExInfo.lpVerb = "runas";
		shExInfo.lpFile = modulePath;
		shExInfo.lpParameters = "/update";
		shExInfo.lpDirectory = sModulePath.substr(0, lastDelim).c_str();
		shExInfo.nShow = SW_SHOW;
		shExInfo.hInstApp = nullptr;

		if (ShellExecuteEx(&shExInfo)) {
			WaitForSingleObject(shExInfo.hProcess, INFINITE);
			CloseHandle(shExInfo.hProcess);
		}

		FreeUpdateInformationSharedMemory();

		return true;
	}

	bool Updater::PerformPreparedUpdate() {
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

	bool Updater::WriteUpdateInformationToSharedMemory() {
		versionToUpdateToMapping = CreateFileMapping(INVALID_HANDLE_VALUE,
			nullptr, PAGE_READWRITE, 0, DWORD(4 * sizeof(short)),
			"inetrUpdateVersionMapping");
		if (versionToUpdateToMapping == nullptr || versionToUpdateToMapping ==
			INVALID_HANDLE_VALUE)
			return false;

		LPVOID versionMappingPtr = MapViewOfFile(versionToUpdateToMapping,
			FILE_MAP_ALL_ACCESS, 0, 0, DWORD(4 * sizeof(short)));
		if (versionMappingPtr == nullptr) {
			CloseHandle(versionToUpdateToMapping);
			return false;
		}

		memcpy(versionMappingPtr, (void*)versionToUpdateTo, 4 * sizeof(short));
		UnmapViewOfFile(versionMappingPtr);

		size_t filesToDlMappingSize = 0;
		for (vector<string>::iterator it = remoteFilesToDownload.begin();
			it != remoteFilesToDownload.end(); ++it) {

				filesToDlMappingSize += it->length() + 1;
		}
		filesToDlMappingSize += 1;

		remoteFilesToDownloadSizeMapping = CreateFileMapping(
			INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(size_t),
			"inetrUpdateMappingSize");
		if (remoteFilesToDownloadSizeMapping == nullptr ||
			remoteFilesToDownloadSizeMapping == INVALID_HANDLE_VALUE)
			return false;

		LPVOID sizeMappingPtr = MapViewOfFile(remoteFilesToDownloadSizeMapping,
			FILE_MAP_ALL_ACCESS, 0, 0, sizeof(size_t));
		if (sizeMappingPtr == nullptr) {
			CloseHandle(remoteFilesToDownloadSizeMapping);
			return false;
		}

		memcpy(sizeMappingPtr, &filesToDlMappingSize, sizeof(size_t));
		UnmapViewOfFile(sizeMappingPtr);

		remoteFilesToDownloadMapping = CreateFileMapping(
			INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE,
			(DWORD)(((unsigned long long)filesToDlMappingSize &
			0xffffffff00000000) >> 4),
			(DWORD)((unsigned long long)filesToDlMappingSize &
			0x00000000ffffffff), "inetrUpdateMapping");
		if (remoteFilesToDownloadMapping == nullptr ||
			remoteFilesToDownloadMapping == INVALID_HANDLE_VALUE)
			return false;

		LPVOID filesToDlPtr = MapViewOfFile(remoteFilesToDownloadMapping,
			FILE_MAP_ALL_ACCESS, 0, 0, filesToDlMappingSize);
		if (filesToDlPtr == nullptr) {
			CloseHandle(remoteFilesToDownloadMapping);
			return false;
		}

		char *ptr = (char*)filesToDlPtr;
		for (vector<string>::iterator it = remoteFilesToDownload.begin();
			it != remoteFilesToDownload.end(); ++it) {

				memcpy(ptr, it->c_str(), it->length() + 1);
				ptr += (ptrdiff_t)(it->length() + 1);
		}
		*ptr = '\0';

		UnmapViewOfFile(filesToDlPtr);

		return true;
	}

	bool Updater::FetchUpdateInformationFromSharedMemory() {
		HANDLE versionMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,
			"inetrUpdateVersionMapping");
		if (versionMapping == nullptr || versionMapping == INVALID_HANDLE_VALUE)
			return false;

		LPVOID versionMappingPtr = MapViewOfFile(versionMapping,
			FILE_MAP_ALL_ACCESS, 0, 0, 4 * sizeof(short));
		if (versionMappingPtr == nullptr) {
			CloseHandle(versionMapping);
			return false;
		}

		memcpy(versionToUpdateTo, versionMappingPtr, 4 * sizeof(short));
		UnmapViewOfFile(versionMappingPtr);
		CloseHandle(versionMapping);

		HANDLE sizeMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,
			"inetrUpdateMappingSize");
		if (sizeMapping == nullptr || sizeMapping == INVALID_HANDLE_VALUE)
			return false;

		LPVOID sizeMappingPtr = MapViewOfFile(sizeMapping, FILE_MAP_ALL_ACCESS,
			0, 0, sizeof(size_t));
		if (sizeMappingPtr == nullptr) {
			CloseHandle(sizeMapping);
			return false;
		}

		size_t filesToDlMappingSize = 0;
		memcpy(&filesToDlMappingSize, sizeMappingPtr, sizeof(size_t));
		UnmapViewOfFile(sizeMappingPtr);
		CloseHandle(sizeMapping);

		HANDLE filesToDlMapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE,
			"inetrUpdateMapping");
		if (filesToDlMapping == nullptr || filesToDlMapping ==
			INVALID_HANDLE_VALUE)
			return false;

		LPVOID filesToDlPtr = MapViewOfFile(filesToDlMapping,
			FILE_MAP_ALL_ACCESS, 0, 0, filesToDlMappingSize);
		if (filesToDlPtr == nullptr) {
			CloseHandle(filesToDlMapping);
			return false;
		}

		char *ptr = (char*)filesToDlPtr;
		while (*ptr != '\0') {
			remoteFilesToDownload.push_back(string(ptr));
			ptr += (ptrdiff_t)(strlen(ptr) + 1);
		}

		UnmapViewOfFile(filesToDlPtr);
		CloseHandle(filesToDlMapping);

		return true;
	}

	void Updater::FreeUpdateInformationSharedMemory() {
		CloseHandle(versionToUpdateToMapping);
		CloseHandle(remoteFilesToDownloadSizeMapping);
		CloseHandle(remoteFilesToDownloadMapping);
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