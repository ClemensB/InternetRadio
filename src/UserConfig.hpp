#ifndef INETR_USERCONFIG_HPP
#define INETR_USERCONFIG_HPP

#include <list>

#include "Language.hpp"
#include "Languages.hpp"
#include "Station.hpp"
#include "Stations.hpp"

namespace inetr {
	class UserConfig {
	public:
		UserConfig(Stations &stations, Languages &languages) :
			CurrentLanguage(languages.DefaultLanguage),
			stations(stations),
			languages(languages) { RadioVolume = 1.0f; }

		bool Load();
		bool Save();


		std::list<const Station*> FavoriteStations;
		float RadioVolume;
		Language &CurrentLanguage;
	private:
		Languages &languages;
		Stations &stations;
	};
}

#endif  // !INETR_USERCONFIG_HPP
