#ifndef INETR_METAMETADATAPROVIDER
#define INETR_METAMETADATAPROVIDER

#include "MetadataProvider.hpp"

namespace inetr {
	class MetaMetadataProvider : public MetadataProvider {
	public:
		MetaMetadataProvider() : MetadataProvider(std::string("meta"), 0) { }
		~MetaMetadataProvider() { }

		std::string Fetch(HSTREAM stream,
			std::map<std::string, std::string> &additionalParameterValues);
	};
}

#endif // !INETR_METAMETADATAPROVIDER