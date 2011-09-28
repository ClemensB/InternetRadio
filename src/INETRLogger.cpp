#include "INETRLogger.hpp"

#include <ctime>
#include <fstream>

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

		ofstream logFile;
		logFile.open("InternetRadio.log", ios::out | ios::app);

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