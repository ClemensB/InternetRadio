#ifndef INETR_REGEXMETADATAPROCESSOR
#define INETR_REGEXMETADATAPROCESSOR

#include "MetadataProcessor.hpp"

namespace inetr {
	class RegExMetadataProcessor : public MetadataProcessor {
	public:
		RegExMetadataProcessor() : MetadataProcessor(std::string("regex"), 1,
			"regex") { }
		~RegExMetadataProcessor() { }

		void Process(std::string &meta, std::map<std::string, std::string>
			&additionalParameterValues);
	};
}

#endif // !INETR_REGEXMETADATAPROCESSOR