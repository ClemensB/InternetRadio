#ifndef INETR_LANGUAGES_HPP
#define INETR_LANGUAGES_HPP

#include <vector>

#include "Language.hpp"

namespace inetr {
	class Languages {
	public:
		Languages() : DefaultLanguage(Languages::None) { };

		bool Load();

		bool IsLanguageLoaded(const std::string &identifier) const;

		const Language &operator[](const std::string &identifier) const;

		inline std::vector<Language>::const_iterator begin() const {
			return languages.begin();
		}
		inline std::vector<Language>::const_iterator end() const {
			return languages.end();
		}

		Language &DefaultLanguage;

		static Language None;
	private:
		std::vector<Language> languages;
	};
}

#endif // !INETR_LANGUAGES_HPP