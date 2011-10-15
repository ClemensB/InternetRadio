#ifndef INETR_HTTPMETASOURCE_HPP
#define INETR_HTTPMETASOURCE_HPP

#include "MetaSource.hpp"

namespace inetr {
	class HTTPMetaSource : public MetaSource {
	public:
		HTTPMetaSource() : MetaSource("http") { }
		~HTTPMetaSource() { }

		bool Get(std::map<std::string, std::string> &parameters, std::string
			&out);
	};
}

#endif // !INETR_HTTPMETASOURCE_HPP