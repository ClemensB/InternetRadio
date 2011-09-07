#ifndef INETR_HTMLENTITYFIXMETADATAPROCESSOR
#define INETR_HTMLENTITYFIXMETADATAPROCESSOR

#include "MetadataProcessor.hpp"

namespace inetr {
	class HTMLEntityFixMetadataProcessor : public MetadataProcessor {
	private:
		static const char* const entities[][2];
		static const int entityCount;
	public:
		HTMLEntityFixMetadataProcessor() : MetadataProcessor(
			std::string("htmlEntityFix"), 0) { }
		~HTMLEntityFixMetadataProcessor() { }

		void Process(std::string &meta, std::map<std::string, std::string>
			&additionalParameterValues);
	};
}

#endif // !INETR_HTMLENTITYFIXMETADATAPROCESSOR