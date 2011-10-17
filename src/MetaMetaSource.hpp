#ifndef INETR_METAMETASOURCE_HPP
#define INETR_METAMETASOURCE_HPP

#include "MetaSourcePrototype.hpp"

namespace inetr {
	class MetaMetaSource : public MetaSourcePrototype {
	public:
		MetaMetaSource() : MetaSourcePrototype("meta") { }
		~MetaMetaSource() { }

		bool Get(const std::map<std::string, std::string> &parameters,
			std::vector<std::string> &precedingMetaSources, std::string &out)
			const;
	};
}

#endif // !INETR_METAMETASOURCE_HPP