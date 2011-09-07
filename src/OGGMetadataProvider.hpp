#ifndef INETR_OGGMETADATAPROVIDER
#define INETR_OGGMETADATAPROVIDER

#include "MetadataProvider.hpp"

namespace inetr {
	class OGGMetadataProvider : public MetadataProvider {
	public:
		OGGMetadataProvider() : MetadataProvider(std::string("ogg"), 0) { }
		~OGGMetadataProvider() { }

		std::string Fetch(HSTREAM stream,
			std::map<std::string, std::string> &additionalParameterValues);
	};
}

#endif // !INETR_OGGMETADATAPROVIDER