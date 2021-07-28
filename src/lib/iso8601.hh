#include <chrono>
#include <cstddef>
#include "visibility.h"

namespace pv {
namespace utils {

extern std::chrono::system_clock::time_point
iso8601(const char *d, std::size_t n,
	std::chrono::system_clock::duration& off) DSO_LOCAL;

extern std::chrono::system_clock::time_point
iso8601(const char *d, std::size_t n) DSO_LOCAL;

}
}
