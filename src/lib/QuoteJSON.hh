#pragma once

#include <string>
#include "visibility.h"

namespace pv {
namespace utils {

class DSO_LOCAL QuoteJSON;
class QuoteJSON {
public:
	static std::string quote(const std::string&);
	static std::string quote(const char *);
	static std::string unquote(const std::string&);
	static std::string unquote(const char *);
};

}
}
