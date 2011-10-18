#ifndef INETR_INETRLOGGER_HPP
#define INETR_INETRLOGGER_HPP

#include <string>

namespace inetr {
	class INETRLogger {
	private:
		void initializeLoggingSession();
		void shutdownLoggingSession();

		std::string getTimestamp();
		void log(std::string str);

		bool isLogging;


		static INETRLogger instance;
	public:
		INETRLogger();
		~INETRLogger();

		void LogInfo(std::string str);
		void LogError(std::string str);


		static INETRLogger *GetInstance();
	};
}

#endif  // !INETR_INETRLOGGER_HPP
