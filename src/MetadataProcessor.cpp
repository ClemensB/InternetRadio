#include "MetadataProcessor.hpp"

#include <cstdarg>

using namespace std;

namespace inetr {
	MetadataProcessor::MetadataProcessor(string identifier,
		int additionalParameterCount, ...) {
		
		this->identifier = identifier;

		va_list vl;
		va_start(vl, additionalParameterCount);
		for (int i = 0; i < additionalParameterCount; ++i) {
			const char* parameter = va_arg(vl, const char*);
			string sParameter(parameter);
			bool optional = sParameter.compare(0, 1, "_") == 0;
			if (optional) sParameter = sParameter.substr(1);
			additionalParameters.insert(pair<string, bool>(sParameter,
				optional));
		}
		va_end(vl);
	}

	string& MetadataProcessor::GetIdentifier() {
		return identifier;
	}

	map<string, bool>* MetadataProcessor::GetAdditionalParameters() {
		return &additionalParameters;
	}
}