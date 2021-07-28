#include "pv/Errors.hh"
#include "pv/Exception.hh"
#include "visibility.h"
#include <syslog.h>

namespace pv {

class DSO_LOCAL Ex;
class Ex: public Exception {
public:
	static void init() __attribute__((constructor));
};

void
Ex::init() {
	ErrorMessages& msgs = Exception::messages();
	msgs.insert({
		{error::OOM,          {LOG_CRIT, "Oom"}},
		{error::ZLIB,         {LOG_ERR, "Zlib internal error"}},
		{error::CRYPTO,       {LOG_ERR, "libcrypto error"}},
		{error::INVALID_ARGS, {LOG_INFO, "Invalid arguments"}},
		{error::INTERNAL,     {LOG_ERR, "Invalid arguments"}},
		{error::CURL,         {LOG_ERR, "libcurl error"}},
	});
}

}
