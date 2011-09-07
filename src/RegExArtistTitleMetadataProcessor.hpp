#ifndef INETR_REGEXARTISTTITLEMETADATAPROCESSOR
#define INETR_REGEXARTISTTITLEMETADATAPROCESSOR

#include "MetadataProcessor.hpp"

namespace inetr {
	class RegExArtistTitleMetadataProcessor : public MetadataProcessor {
	public:
		RegExArtistTitleMetadataProcessor() : MetadataProcessor(
			std::string("regexAT"), 2, "regexA", "regexT") { }
		~RegExArtistTitleMetadataProcessor() { }

		void Process(std::string &meta, std::map<std::string, std::string>
			&additionalParameterValues);
	};
}

#endif // !INETR_REGEXARTISTTITLEMETADATAPROCESSOR