#ifndef INETR_METASOURCE_HPP
#define INETR_METASOURCE_HPP

#include <string>
#include <map>

namespace inetr {
	class MetaSource {
	public:
		virtual ~MetaSource() = 0;

		inline std::string &GetIdentifer() { return identifier; }

		virtual bool Get(std::map<std::string, std::string> &parameters,
			std::string &out) = 0;
	protected:
		MetaSource(std::string identifier) { this->identifier = identifier; }
	private:
		std::string identifier;
	};
}

#endif // !INETR_METASOURCE_HPP