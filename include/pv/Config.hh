#pragma once

#include <string>

#include "internal/conf_macros_hdr.h"

namespace pv {

class Config {
private:
	template<int n>
	struct ___Constants {
		static const std::string DFLT_URL;
	};

public:
	typedef ___Constants<0> Constants;

private:
	FIELD_DECLARATIONS(url, std::string);
	FIELD_DECLARATIONS(key, std::string);
	FIELD_DECLARATIONS(cert, std::string);

public:
	Config();

	METHOD_DECLARATIONS(url, const std::string&);
	METHOD_DECLARATIONS(key, const std::string&);
	METHOD_DECLARATIONS(cert, const std::string&);

	void merge(const Config&);
};

template<int n>
const std::string Config::___Constants<n>::DFLT_URL("https://dgc-verification-prod.incert.lu:9443/api/get-certificates");

METHOD_DEFINITIONS(url, Config, const std::string&)
METHOD_DEFINITIONS(key, Config, const std::string&)
METHOD_DEFINITIONS(cert, Config, const std::string&)

inline
Config::Config()
	: FIELD_INIT(url, Constants::DFLT_URL)
	, FIELD_INIT(key, std::string())
	, FIELD_INIT(cert, std::string())
{
}

inline void
Config::merge(const Config& parent) {
	MERGE_VAR(url, &parent);
	MERGE_VAR(key, &parent);
	MERGE_VAR(cert, &parent);
}

}

#include "internal/conf_macros_footer.h"
