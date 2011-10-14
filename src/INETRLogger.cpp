#include "INETRLogger.hpp"

#include <ctime>
#include <fstream>

#include <Windows.h>
#include <ShlObj.h>

using namespace std;

namespace inetr {
	INETRLogger INETRLogger::instance;


	INETRLogger::INETRLogger() {
		initializeLoggingSession();
	}

	INETRLogger::~INETRLogger() {
		shutdownLoggingSession();
	}

	void INETRLogger::initializeLoggingSession() {
		if (isLogging)
			return;

		isLogging = true;

		log("Started logging session");
	}

	void INETRLogger::shutdownLoggingSession() {
		if (!isLogging)
			return;

		isLogging = false;

		log("Stopped logging session");
	}

	string INETRLogger::getTimestamp() {
		time_t rawtime;

		time(&rawtime);
		char buf[30];
		ctime_s(buf, sizeof(buf), &rawtime);
		return string(buf);
	}

	void INETRLogger::log(string str) {
#ifndef DEBUG
		return;
#endif

		char appDataPath[MAX_PATH];
		SHGetFolderPath(nullptr, CSIDL_APPDATA, nullptr, SHGFP_TYPE_CURRENT,
			appDataPath);

		string inetrDir = string(appDataPath) + "\\InternetRadio";

		if (GetFileAttributes(inetrDir.c_str()) == INVALID_FILE_ATTRIBUTES)
			CreateDirectory(inetrDir.c_str(), nullptr);

		ofstream logFile;
		logFile.open(inetrDir + "\\InternetRadio.log", ios::out | ios::app);

		logFile << getTimestamp() << str << endl;

		logFile.close();
	}
	
	void INETRLogger::LogInfo(string str) {
		log(string("INFO: ") + str);
	}

	void INETRLogger::LogError(string str) {
		log (string("ERROR: ") + str);
	}


	INETRLogger *INETRLogger::GetInstance() {
		return &instance;
	}
}