#ifndef INETR_METASOURCEPROTOTYPE_HPP
#define INETR_METASOURCEPROTOTYPE_HPP

#include <map>
#include <string>
#include <vector>

namespace inetr {
	class MetaSourcePrototype {
	public:
		virtual ~MetaSourcePrototype() { }

		inline std::string &GetIdentifer() { return identifier; }

		virtual bool Get(const std::map<std::string, std::string> &parameters,
			std::vector<std::string> &precedingMetaSources, std::string &out)
			const = 0;
	protected:
		MetaSourcePrototype(std::string identifier) { this->identifier =
			identifier; }
	private:
		std::string identifier;
	};
}

#endif  // !INETR_METASOURCEPROTOTYPE_HPP
