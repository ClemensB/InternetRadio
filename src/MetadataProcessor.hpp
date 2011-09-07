#ifndef INETR_METADATAPROCESSOR_HPP
#define INETR_METADATAPROCESSOR_HPP

#include <string>
#include <list>
#include <map>

namespace inetr {
	class MetadataProcessor {
	private:
		std::string identifier;
		std::map<std::string, bool> additionalParameters;
	public:
		MetadataProcessor(std::string identifier, int additionalParameterCount,
			...);
		virtual ~MetadataProcessor() { }

		std::string& GetIdentifier();
		std::map<std::string, bool>* GetAdditionalParameters();

		virtual void Process(std::string &meta, std::map<std::string,
			std::string> &additionalParameterValues) = 0;
	};
}

#endif // !INETR_METADATAPROCESSOR_HPP