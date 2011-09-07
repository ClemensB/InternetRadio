#ifndef INETR_METADATAPROVIDER_HPP
#define INETR_METADATAPROVIDER_HPP

#include <string>
#include <list>
#include <map>

#include <bass.h>

namespace inetr {
	class MetadataProvider {
	private:
		std::string identifier;
		std::list<std::string> additionalParameters;
	public:
		MetadataProvider(std::string identifier, int additionalParameterCount,
			...);
		virtual ~MetadataProvider() { }

		std::string& GetIdentifier();
		std::list<std::string>* GetAdditionalParameters();

		virtual std::string Fetch(HSTREAM stream,
			std::map<std::string, std::string> &additionalParameterValues) = 0;
	};
}

#endif // !INETR_METADATAPROVIDER_HPP