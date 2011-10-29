#ifndef INETR_HTTPMETASOURCE_HPP
#define INETR_HTTPMETASOURCE_HPP

#include <map>
#include <string>
#include <vector>

#include "MetaSourcePrototype.hpp"

namespace inetr {
	class HTTPMetaSource : public MetaSourcePrototype {
	public:
		HTTPMetaSource() : MetaSourcePrototype("http") { }
		~HTTPMetaSource() { }

		bool Get(const std::map<std::string, std::string> &parameters,
			std::vector<std::string> &precedingMetaSources, std::string &out)
			const;
	};
}

#endif  // !INETR_HTTPMETASOURCE_HPP
