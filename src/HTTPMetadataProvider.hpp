#ifndef INETR_HTTPMETADATAPROVIDER
#define INETR_HTTPMETADATAPROVIDER

#include "MetadataProvider.hpp"

namespace inetr {
	class HTTPMetadataProvider : public MetadataProvider {
	public:
		HTTPMetadataProvider() : MetadataProvider(std::string("http"), 1,
			"httpURL") { }
		~HTTPMetadataProvider() { }

		std::string Fetch(HSTREAM stream,
			std::map<std::string, std::string> &additionalParameterValues);
	};
}

#endif // !INETR_HTTPMETADATAPROVIDER