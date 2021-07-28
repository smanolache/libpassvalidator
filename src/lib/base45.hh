#pragma once

#include "ustring.hh"
#include <string>
#include "visibility.h"
#include <cstddef>

namespace pv {
namespace utils {

extern ustring base45_dec(const std::string&) DSO_LOCAL;
extern ustring base45_dec(const char *, const char *) DSO_LOCAL;
extern ustring base45_dec(const char *, std::size_t) DSO_LOCAL;

}
}
