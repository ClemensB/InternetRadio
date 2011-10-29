#ifndef INETR_STATIONS_HPP
#define INETR_STATIONS_HPP

#include <list>

#include "MetaSourcePrototype.hpp"
#include "Station.hpp"

namespace inetr {
	class Stations {
	public:
		Stations();
		~Stations();

		bool Load();

		inline std::list<Station>::const_iterator begin() const {
			return stations.begin();
		}
		inline std::list<Station>::const_iterator end() const {
			return stations.end();
		}

		std::list<MetaSourcePrototype*> MetaSourcePrototypes;
	private:
		std::list<Station> stations;
	};
}

#endif  // !INETR_STATIONS_HPP
