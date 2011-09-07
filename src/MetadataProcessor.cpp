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
			additionalParameters.push_back(string(parameter));
		}
		va_end(vl);
	}

	string& MetadataProcessor::GetIdentifier() {
		return identifier;
	}

	list<string>* MetadataProcessor::GetAdditionalParameters() {
		return &additionalParameters;
	}
}