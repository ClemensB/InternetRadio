#include "Updater.hpp"

#include <cstdint>

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <Windows.h>

#include "../resource/resource.h"

#include "CryptUtil.hpp"
#include "HTTP.hpp"
#include "INETRException.hpp"
#include "MUtil.hpp"
#include "StringUtil.hpp"
#include "VersionUtil.hpp"

using std::ifstream;
using std::ios;
using std::vector;
using std::map;
using std::ofstream;
using std::pair;
using std::string;
using std::stringstream;

namespace inetr {
	Updater::Updater(string remoteUpdateRoot) {
		this->remoteUpdateRoot = remoteUpdateRoot;
	}

	bool Updater::GetRemoteVersion(uint16_t *version) {
		stringstream versionFileStream;
		try {
			HTTP::Get(remoteUpdateRoot + "/version", &versionFileStream);
		} catch(INETRException) {
			return false;
		}

		string versionStr;
		versionFileStream >> versionStr;
		VersionUtil::VersionStrToArr(versionStr, version);
		return (versionStr != "");
	}

	bool Updater::IsInstalledVersionUpToDate(bool &isUpToDate, uint16_t
		*upToDateVersion) {
		uint16_t installedVersion[4], remoteVersion[4];
		if (!VersionUtil::GetInstalledVersion(installedVersion))
			return false;
		if (!GetRemoteVersion(remoteVersion))
			return false;
		if (isUpToDate = (VersionUtil::CompareVersions(remoteVersion,
			installedVersion) != VCR_Newer)) {

			upToDateVersion = nullptr;
		} else {
			memcpy(upToDateVersion, remoteVersion, sizeof(remoteVersion));
		}
		return true;
	}

	bool Updater::ReceiveRemoteVersionInfo(uint16_t *version,
		stringstream &info) {

		string versionStr;
		VersionUtil::VersionArrToStr(version, versionStr, true);

		try {
			HTTP::Get(remoteUpdateRoot + "/" + versionStr + "/changelog",
				&info);
		} catch(INETRException) {
			return false;
		}

		return true;
	}

	bool Updater::PrepareUpdateToRemoteVersion(uint16_t *version) {
		memset(versionToUpdateTo, 0, sizeof(versionToUpdateTo));
		remoteFilesToDownload.clear();

		string versionStr;
		VersionUtil::VersionArrToStr(version, versionStr, true);

		stringstream remoteChecksumsStream;
		try {
			HTTP::Get(remoteUpdateRoot + "/" + versionStr + "/" + INETR_ARCH +
				"/checksums", &remoteChecksumsStream);
		} catch(INETRException) {
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
				} catch(INETRException) {
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
		if (reinterpret_cast<uint64_t>(versionToUpdateTo) == 0L ||
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
			VersionUtil::VersionArrToStr(versionToUpdateTo, versionStr, true);

			string remoteURL = remoteUpdateRoot + "/" + versionStr + "/" +
				INETR_ARCH + "/" + remoteFilename;

			string localTmpFilename = localFilename + ".updatetmp";
			DeleteFile(localTmpFilename.c_str());
			MoveFile(localFilename.c_str(), localTmpFilename.c_str());

			stringstream remoteFileStream;
			try {
				HTTP::Get(remoteURL, &remoteFileStream);
			} catch(INETRException) {
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
			nullptr, PAGE_READWRITE, 0, DWORD(4 * sizeof(uint16_t)),
			"inetrUpdateVersionMapping");
		if (versionToUpdateToMapping == nullptr || versionToUpdateToMapping ==
			INVALID_HANDLE_VALUE)
			return false;

		LPVOID versionMappingPtr = MapViewOfFile(versionToUpdateToMapping,
			FILE_MAP_ALL_ACCESS, 0, 0, 4 * sizeof(uint16_t));
		if (versionMappingPtr == nullptr) {
			CloseHandle(versionToUpdateToMapping);
			return false;
		}

		memcpy(versionMappingPtr, reinterpret_cast<void*>(versionToUpdateTo), 4
			* sizeof(uint16_t));
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
			(DWORD)(((uint64_t)filesToDlMappingSize &
			0xffffffff00000000) >> 4),
			(DWORD)((uint64_t)filesToDlMappingSize &
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

		char *ptr = reinterpret_cast<char*>(filesToDlPtr);
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
			FILE_MAP_ALL_ACCESS, 0, 0, 4 * sizeof(uint16_t));
		if (versionMappingPtr == nullptr) {
			CloseHandle(versionMapping);
			return false;
		}

		memcpy(versionToUpdateTo, versionMappingPtr, 4 * sizeof(uint16_t));
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

		char *ptr = reinterpret_cast<char*>(filesToDlPtr);
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
}
