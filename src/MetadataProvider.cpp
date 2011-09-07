#include "MetadataProvider.hpp"

#include <cstdarg>

using namespace std;

namespace inetr {
	MetadataProvider::MetadataProvider(string identifier,
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

	string& MetadataProvider::GetIdentifier() {
		return identifier;
	}

	list<string>* MetadataProvider::GetAdditionalParameters() {
		return &additionalParameters;
	}
}