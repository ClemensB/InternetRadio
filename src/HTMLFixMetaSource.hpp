#ifndef INETR_HTMLFIXMETASOURCE_HPP
#define INETR_HTMLFIXMETASOURCE_HPP

#include "MetaSourcePrototype.hpp"

namespace inetr {
	class HTMLFixMetaSource : public MetaSourcePrototype {
	public:
		HTMLFixMetaSource() : MetaSourcePrototype("htmlFix") { }
		~HTMLFixMetaSource() { }

		bool Get(const std::map<std::string, std::string> &parameters,
			std::vector<std::string> &precedingMetaSources, std::string &out)
			const;
	private:
		static const char* const entities[][2];
		static const size_t entityCount;
	};
}

#endif // !INETR_HTMLFIXMETASOURCE_HPP