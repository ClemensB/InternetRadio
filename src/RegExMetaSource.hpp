#ifndef INETR_REGEXMETASOURCE_HPP
#define INETR_REGEXMETASOURCE_HPP

#include "MetaSourcePrototype.hpp"

namespace inetr {
	class RegExMetaSource : public MetaSourcePrototype {
	public:
		RegExMetaSource() : MetaSourcePrototype("regex") { }
		~RegExMetaSource() { }

		bool Get(const std::map<std::string, std::string> &parameters,
			std::vector<std::string> &precedingMetaSources, std::string &out)
			const;
	};
}

#endif // !INETR_REGEXMETASOURCE_HPP