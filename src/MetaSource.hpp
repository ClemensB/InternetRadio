#ifndef INETR_METASOURCE_HPP
#define INETR_METASOURCE_HPP

#include <algorithm>
#include <string>
#include <map>

#include "MetaSourcePrototype.hpp"

namespace inetr {
	class MetaSource {
	public:
		MetaSource(MetaSourcePrototype *metaSourceProto, std::map<std::string,
			std::string> parameters) {

			MetaSourceProto = metaSourceProto;
			Parameters = parameters;
		}

		inline bool Get(std::vector<std::string> &precedingMetaSources,
			std::string &out, std::map<std::string, std::string>
			&additionalParameters = std::map<std::string, std::string>())
			const {

			std::map<std::string, std::string> mParams;
			mParams.insert(Parameters.begin(), Parameters.end());
			mParams.insert(additionalParameters.begin(),
				additionalParameters.end());

			return MetaSourceProto->Get(mParams, precedingMetaSources, out);
		}

		MetaSourcePrototype *MetaSourceProto;

		std::map<std::string, std::string> Parameters;
	};
}

#endif // !INETR_METASOURCE_HPP